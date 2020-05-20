/*
 * DRace, a dynamic data race detector
 *
 * Copyright 2018 Siemens AG
 *
 * Authors:
 *   Felix Moessbauer <felix.moessbauer@siemens.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "globals.h"

#include <dr_api.h>
#include <dr_tools.h>
#include <drmgr.h>
#include <drreg.h>
#include <drutil.h>
#include <hashtable.h>

#include <detector/Detector.h>

#include "MSR.h"
#include "Module.h"
#include "function-wrapper.h"
#include "ipc/DrLock.h"
#include "memory-tracker.h"
#include "shadow-stack.h"
#include "statistics.h"
#include "symbols.h"

#if WINDOWS
#include "ipc/SMData.h"
#include "ipc/SharedMemory.h"
#endif

#include <mutex>  // for lock_guard

namespace drace {

MemoryTracker::MemoryTracker(const std::shared_ptr<Statistics> &stats)
    : _prng(static_cast<unsigned>(std::chrono::high_resolution_clock::now()
                                      .time_since_epoch()
                                      .count())),
      _stats(stats) {
  /* We need 3 reg slots beyond drreg's eflags slots => 3 slots */
  drreg_options_t ops = {sizeof(ops), 4, false};

  /* Ensure that atomic and native type have equal size as otherwise
     instrumentation reads invalid value */
  using no_flush_t = decltype(ShadowThreadState::no_flush);

#ifdef WINDOWS
  // this requires C++17, but is available in MSVC with C++11
  static_assert(sizeof(no_flush_t) == sizeof(no_flush_t::value_type),
                "atomic uint64 size differs from uint64 size");
#endif

  DR_ASSERT(drreg_init(&ops) == DRREG_SUCCESS);
  page_size = dr_page_size();

  // Prepare vector with allowed registers for REG2
  drreg_init_and_fill_vector(&allowed_xcx, false);
  drreg_set_vector_entry(&allowed_xcx, DR_REG_XCX, true);

  // Initialize Code Caches
  code_cache_init();

  // setup sampling
  update_sampling();

  DR_ASSERT(drmgr_register_bb_app2app_event(instr_event_bb_app2app, NULL) &&
            drmgr_register_bb_instrumentation_event(
                instr_event_app_analysis, instr_event_app_instruction, NULL));

  LOG_INFO(0, "Initialized");
}

MemoryTracker::~MemoryTracker() {
  dr_nonheap_free(cc_flush, page_size);

  drvector_delete(&allowed_xcx);

  if (!drmgr_unregister_bb_app2app_event(instr_event_bb_app2app) ||
      !drmgr_unregister_bb_instrumentation_event(instr_event_app_analysis) ||
      drreg_exit() != DRREG_SUCCESS)
    DR_ASSERT(false);
}

inline void flush_region(void *drcontext, uintptr_t pc) {
  // Flush this area from the code cache
  dr_delay_flush_region((app_pc)(pc << MemoryTracker::HIST_PC_RES),
                        1 << MemoryTracker::HIST_PC_RES, 0, NULL);
  LOG_NOTICE(dr_get_thread_id(drcontext), "Flushed %p",
             pc << MemoryTracker::HIST_PC_RES);
}

inline std::vector<uintptr_t> get_pcs_from_hist(
    const Statistics::hist_t &hist) {
  std::vector<uintptr_t> result;
  result.reserve(hist.size());

  std::transform(
      hist.begin(), hist.end(), std::back_inserter(result),
      [](const Statistics::hist_t::value_type &v) { return v.first; });

  std::sort(result.begin(), result.end());

  return result;
}

void MemoryTracker::update_cache(ShadowThreadState &data) {
  // TODO: optimize this
  const auto &new_freq = data.stats.pc_hits.computeOutput<Statistics::hist_t>();
  LOG_NOTICE(data.tid, "Flush Cache with size %i", new_freq.size());
  if (params.lossy_flush) {
    void *drcontext = dr_get_current_drcontext();
    const auto &pc_new = get_pcs_from_hist(new_freq);
    const auto &pc_old = data.stats.freq_pcs;

    std::vector<uintptr_t> difference;
    difference.reserve(pc_new.size() + pc_old.size());

    // get difference between both histograms
    std::set_symmetric_difference(pc_new.begin(), pc_new.end(), pc_old.begin(),
                                  pc_old.end(), std::back_inserter(difference));

    // Flush new fragments
    for (const auto pc : difference) {
      flush_region(drcontext, pc);
    }
    data.stats.freq_pcs = pc_new;
  }
  data.stats.freq_pc_hist = new_freq;
}

bool MemoryTracker::pc_in_freq(ShadowThreadState &data, void *bb) {
  const auto &freq_pcs = data.stats.freq_pcs;
  return std::binary_search(freq_pcs.begin(), freq_pcs.end(),
                            ((uintptr_t)bb >> MemoryTracker::HIST_PC_RES));
}

void MemoryTracker::analyze_access(ShadowThreadState &data) {
  if (data.detector_data == nullptr) {
    // Thread starts with a pending clean-call
    // We missed a fork
    // 1. Flush all threads (except this thread)
    // 2. Fork thread
    LOG_TRACE(data.tid, "Missed a fork, do it now");
    detector->fork(runtime_tid.load(std::memory_order_relaxed),
                   static_cast<Detector::tid_t>(data.tid),
                   &(data.detector_data));
    // arc between parent thread and this thread
    detector->happens_after(data.detector_data, (void *)(uintptr_t)(data.tid));
    clear_buffer();
    return;
  }

  // toggle detector on external state change
  // avoid expensive mod by comparing with bitmask
  if ((data.stats.flushes & (0xF - 1)) == (0xF - 1)) {
    // lessen impact of expensive SHM accesses
    memory_tracker->handle_ext_state(data);
  }

  if (data.enabled) {
    mem_ref_t *mem_ref = reinterpret_cast<mem_ref_t *>(data.mem_buf.data());
    uintptr_t num_refs = static_cast<uintptr_t>(
        reinterpret_cast<mem_ref_t *>(data.buf_ptr) - mem_ref);

    if (num_refs > 0) {
      // dr_printf("[%i] Process buffer, noflush: %i, refs: %i\n", data.tid,
      // data.no_flush.load(std::memory_order_relaxed), num_refs);
      DR_ASSERT(data.detector_data != nullptr);

      // Lossy count first mem-ref (all are adiacent as after each call is
      // flushed)
      if (params.lossy) {
        data.stats.pc_hits.processItem(mem_ref->pc >> HIST_PC_RES);
        if ((data.stats.flushes & (CC_UPDATE_PERIOD - 1)) ==
            (CC_UPDATE_PERIOD - 1)) {
          update_cache(data);
        }
      }

      for (; mem_ref < (mem_ref_t *)data.buf_ptr; ++mem_ref) {
        if (params.excl_stack && (mem_ref->addr > data.appstack_beg) &&
            (mem_ref->addr < data.appstack_end)) {
          // this reference points into the stack range, skip
          continue;
        }
        if (mem_ref->addr > PROC_ADDR_LIMIT) {
          // outside process address space
          continue;
        }

        if (mem_ref->write) {
          detector->write(
              data.detector_data, reinterpret_cast<void *>(mem_ref->pc),
              reinterpret_cast<void *>(mem_ref->addr), mem_ref->size);
          // printf("[%i] WRITE %p, PC: %p\n", data.tid, mem_ref->addr,
          // mem_ref->pc);
        } else {
          detector->read(
              data.detector_data, reinterpret_cast<void *>(mem_ref->pc),
              reinterpret_cast<void *>(mem_ref->addr), mem_ref->size);
          // printf("[%i] READ  %p, PC: %p\n", data.tid, mem_ref->addr,
          // mem_ref->pc);
        }
        ++(data.stats.proc_refs);
      }
      data.stats.total_refs += num_refs;
    }
  }
  data.buf_ptr = data.mem_buf.data();
}

/*
 * Thread init Event
 */
void MemoryTracker::event_thread_init(void *drcontext,
                                      ShadowThreadState &data) {
  data.mem_buf.resize(MEM_BUF_SIZE, drcontext);
  data.buf_ptr = data.mem_buf.data();
  // set buf_end to be negative of address of buffer end for the lea later
  data.buf_end =
      (-1) * reinterpret_cast<intptr_t>(data.mem_buf.data() + MEM_BUF_SIZE);

  // this is the master thread
  if (params.exclude_master && (data.tid == runtime_tid)) {
    disable_scope(data);
  }

#ifdef WINDOWS
  // TODO: emulate this for windows 7, linux
  // determin stack range of this thread
  if (runtime_tid.load(std::memory_order_relaxed) != data.tid) {
    if (!dr_using_app_state(drcontext)) dr_switch_to_app_state(drcontext);
    // dr does not support this natively, so make syscall in app context
    GetCurrentThreadStackLimits(&(data.appstack_beg), &(data.appstack_end));
    LOG_NOTICE(data.tid, "stack from %p to %p", data.appstack_beg,
               data.appstack_end);
  } else {
    // TODO: this lookup cannot be performed on master thread, as state is not
    // valid. See drmem#xxx
    LOG_NOTICE(data.tid, "stack range cannot be detected");
  }
#endif
}

void MemoryTracker::event_thread_exit(void *drcontext,
                                      ShadowThreadState &data) {
  flush_all_threads(data, true, false);

  detector->join(runtime_tid.load(std::memory_order_relaxed),
                 static_cast<Detector::tid_t>(data.tid));

  // as this is a exclusive lock and this is the only place
  // where stats are combined, we use it
  {
    std::lock_guard<DrLock> lg(_tls_rw_mutex);
    *_stats |= data.stats;

    if (params.stats_show) {
      data.stats.print_summary(drace::log_target);
    }
  }

  // Cleanup TLS
  // As we cannot rely on current drcontext here, use provided one
  data.mem_buf.deallocate(drcontext);
}

/* We transform string loops into regular loops so we can more easily
 * monitor every memory reference they make.
 */
dr_emit_flags_t MemoryTracker::event_bb_app2app(void *drcontext, void *tag,
                                                instrlist_t *bb, bool for_trace,
                                                bool translating) {
  if (!drutil_expand_rep_string(drcontext, bb)) {
    DR_ASSERT(false);
    /* in release build, carry on: we'll just miss per-iter refs */
  }
  return DR_EMIT_DEFAULT;
}

dr_emit_flags_t MemoryTracker::event_app_analysis(void *drcontext, void *tag,
                                                  instrlist_t *bb,
                                                  bool for_trace,
                                                  bool translating,
                                                  OUT void **user_data) {
  using INSTR_FLAGS = module::Metadata::INSTR_FLAGS;

  if (for_trace && params.excl_traces) {
    *user_data = (void *)INSTR_FLAGS::STACK;
    return DR_EMIT_DEFAULT;
  }

  INSTR_FLAGS instrument_bb;
  app_pc bb_addr = dr_fragment_app_pc(tag);

  // Lookup module from cache, hit is very likely as adiacent bb's
  // are mostly in the same module
  const module::Metadata *modptr = mc.lookup(bb_addr);
  if (nullptr != modptr) {
    instrument_bb = modptr->instrument;
  } else {
    // TODO: investigate performance impact
    auto s_modptr = module_tracker->get_module_containing(bb_addr);
    modptr = s_modptr.get();
    if (modptr) {
      // bb in known module
      instrument_bb = modptr->instrument;
      mc.update(modptr);
    } else {
      // Module not known
      LOG_TRACE(0, "Module unknown, probably JIT code (%p)", bb_addr);
      instrument_bb = (INSTR_FLAGS)(INSTR_FLAGS::MEMORY | INSTR_FLAGS::STACK);
    }
  }

  // Do not instrument if block is frequent TODO: check correctness
  if (params.lossy_flush && for_trace && instrument_bb) {
    ShadowThreadState &data = thread_state.getSlot(drcontext);
    if (pc_in_freq(data, bb_addr)) {
      instrument_bb = INSTR_FLAGS::NONE;
    }
  }

  // Avoid temporary allocation by using ptr-value directly
  *user_data = (void *)instrument_bb;
  return DR_EMIT_DEFAULT;
}

dr_emit_flags_t MemoryTracker::event_app_instruction(
    void *drcontext, void *tag, instrlist_t *bb, instr_t *instr, bool for_trace,
    bool translating, void *user_data) {
  using INSTR_FLAGS = module::Metadata::INSTR_FLAGS;
  auto instrument_instr =
      (INSTR_FLAGS)(util::unsafe_ptr_cast<uintptr_t>(user_data));
  // we treat all atomic accesses as reads
  bool instr_is_atomic{false};

  if (instr_get_app_pc(instr) == NULL) return DR_EMIT_DEFAULT;

  if (!instr_is_app(instr)) return DR_EMIT_DEFAULT;

  if (instrument_instr & INSTR_FLAGS::STACK) {
    // Instrument ShadowStack TODO: This sometimes crashes in Dotnet modules
    if (funwrap::wrap_generic_call(drcontext, tag, bb, instr, for_trace,
                                   translating, user_data))
      return DR_EMIT_DEFAULT;
  }

  if (!(instrument_instr & INSTR_FLAGS::MEMORY)) return DR_EMIT_DEFAULT;
  bool instr_reads_mem = instr_reads_memory(instr);
  bool instr_writes_mem = instr_writes_memory(instr);
  if (!instr_reads_mem && !instr_writes_mem) return DR_EMIT_DEFAULT;

  if (params.excl_stack) {
    // exclude pop and push
    int opcode = instr_get_opcode(instr);
    if (opcode == OP_pop || opcode == OP_popa || opcode == OP_popf ||
        opcode == OP_push || opcode == OP_pusha || opcode == OP_pushf) {
      return DR_EMIT_DEFAULT;
    }

    // exclude other modifications of stackptr
    if (instr_reads_from_reg(instr, DR_REG_XSP, DR_QUERY_DEFAULT) ||
        instr_writes_to_reg(instr, DR_REG_XSP, DR_QUERY_DEFAULT) ||
        instr_reads_from_reg(instr, DR_REG_XBP, DR_QUERY_DEFAULT) ||
        instr_writes_to_reg(instr, DR_REG_XBP, DR_QUERY_DEFAULT)) {
      return DR_EMIT_DEFAULT;
    }
  }

  // atomic instruction
  if (instr_get_prefix_flag(instr, PREFIX_LOCK)) instr_is_atomic = true;

  // Sampling: Only instrument some instructions
  if (!sample_bb(tag)) {
    return DR_EMIT_DEFAULT;
  }

  if (instr_reads_mem) {
    /* insert code to add an entry for each memory reference opnd */
    for (int i = 0; i < instr_num_srcs(instr); i++) {
      opnd_t src = instr_get_src(instr, i);
      if (opnd_is_memory_reference(src)) {
        instrument_mem(drcontext, bb, instr, src, false);
      }
    }
  }
  if (instr_writes_mem) {
    for (int i = 0; i < instr_num_dsts(instr); i++) {
      opnd_t dst = instr_get_dst(instr, i);
      if (opnd_is_memory_reference(dst)) {
        instrument_mem(drcontext, bb, instr, dst, !instr_is_atomic);
      }
    }
  }

  return DR_EMIT_DEFAULT;
}  // namespace drace

/* clean_call dumps the memory reference info into the analyzer */
void MemoryTracker::process_buffer() {
  void *drcontext = dr_get_current_drcontext();
  ShadowThreadState &data = thread_state.getSlot(drcontext);

  analyze_access(data);
  data.stats.flushes++;
}

void MemoryTracker::clear_buffer() {
  ShadowThreadState &data = thread_state.getSlot();
  mem_ref_t *mem_ref = (mem_ref_t *)data.mem_buf.data();
  uintptr_t num_refs = (uintptr_t)((mem_ref_t *)data.buf_ptr - mem_ref);

  data.stats.proc_refs += num_refs;
  data.stats.flushes++;
  data.buf_ptr = data.mem_buf.data();
  data.no_flush.store(true, std::memory_order_relaxed);
}

/* Request a flush of all non-disabled threads.
 *  \Warning: This function read-locks the TLS mutex
 */
void MemoryTracker::flush_all_threads(ShadowThreadState &data, bool self,
                                      bool flush_external) {
  if (self) process_buffer();
}

void MemoryTracker::code_cache_init() {
  void *drcontext;
  instrlist_t *ilist;
  instr_t *where;
  byte *end;

  drcontext = dr_get_current_drcontext();
  cc_flush = (app_pc)dr_nonheap_alloc(
      page_size, DR_MEMPROT_READ | DR_MEMPROT_WRITE | DR_MEMPROT_EXEC);
  ilist = instrlist_create(drcontext);
  /* The lean procecure simply performs a clean call, and then jump back */
  /* jump back to the DR's code cache */
  where = INSTR_CREATE_jmp_ind(drcontext, opnd_create_reg(DR_REG_XCX));
  instrlist_meta_append(ilist, where);
  /* clean call, save fpstate */
  dr_insert_clean_call(drcontext, ilist, where, (void *)process_buffer, true,
                       0);
  /* Encodes the instructions into memory and then cleans up. */
  end = instrlist_encode(drcontext, ilist, cc_flush, false);
  DR_ASSERT((size_t)(end - cc_flush) < page_size);
  instrlist_clear_and_destroy(drcontext, ilist);
  /* set the memory as just +rx now */
  dr_memory_protect(cc_flush, page_size, DR_MEMPROT_READ | DR_MEMPROT_EXEC);
}

void MemoryTracker::handle_ext_state(ShadowThreadState &data) {
#ifdef WINDOWS
  if (shmdriver) {
    bool shm_ext_state = extcb->get()->enabled.load(std::memory_order_relaxed);
    if (_enable_external.load(std::memory_order_relaxed) != shm_ext_state) {
      LOG_INFO(0, "externally switched state: %s",
               shm_ext_state ? "ON" : "OFF");
      _enable_external.store(shm_ext_state, std::memory_order_relaxed);
      if (!shm_ext_state) {
        funwrap::event::beg_excl_region(data);
      } else {
        funwrap::event::end_excl_region(data);
      }
    }
    // set sampling rate
    unsigned sampling_rate =
        extcb->get()->sampling_rate.load(std::memory_order_relaxed);
    if (sampling_rate != params.sampling_rate) {
      LOG_INFO(0, "externally changed sampling rate to: %i", sampling_rate);
      params.sampling_rate = sampling_rate;
      update_sampling();
    }
  }
#endif
}

void MemoryTracker::update_sampling() {
  unsigned delta;
  if (params.sampling_rate < 10)
    delta = 1;
  else {
    delta = static_cast<unsigned>(0.1 * params.sampling_rate);
  }
  _min_period = std::max(params.sampling_rate - delta, 1u);
  _max_period = params.sampling_rate + delta;
}

}  // namespace drace

// Inline Instrumentation in src/instr
