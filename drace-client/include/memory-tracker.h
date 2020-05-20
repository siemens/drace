#pragma once
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

#include "Module.h"
#include "ShadowThreadState.h"
#include "ipc/DrLock.h"
#include "statistics.h"

#include <dr_api.h>
#include <dr_tools.h>
#include <drmgr.h>
#include <drreg.h>
#include <drutil.h>

#include <atomic>
#include <memory>
#include <random>

namespace drace {
/**
 * \brief Covers application memory tracing.
 *
 * Responsible for adding all instrumentation code (except function wrapping).
 * Callbacks from the code-cache are handled in this class.
 * Memory-Reference analysis is done here.
 */
class MemoryTracker {
 public:
  /// Single memory reference
  struct mem_ref_t {
    uintptr_t addr;
    uintptr_t pc;
    uint32_t size;
    bool write;
  };

  /** Upper limit of process address space according to
   *   https://docs.microsoft.com/en-us/windows/win32/memory/memory-limits-for-windows-releases#memory-limits-for-windows-and-windows-server-releases
   *   This also holds for Linux x64
   *   https://www.kernel.org/doc/Documentation/x86/x86_64/mm.txt
   */
#if COMPILE_X86
  static constexpr uintptr_t PROC_ADDR_LIMIT = 0xBFFFFFFF;
#else
  static constexpr uintptr_t PROC_ADDR_LIMIT = 0x00007FFF'FFFFFFFF;
#endif

  /// Maximum number of references between clean calls
  static constexpr int MAX_NUM_MEM_REFS = 64;
  static constexpr int MEM_BUF_SIZE = sizeof(mem_ref_t) * MAX_NUM_MEM_REFS;

  /// aggregate frequent pc's on this granularity (2^n bytes)
  static constexpr unsigned HIST_PC_RES = 10;

  /// update code-cache after this number of flushes (must be power of two)
  static constexpr unsigned CC_UPDATE_PERIOD = 1024 * 64;

 private:
  /// \brief external change detected
  /// this flag is used to trigger the enable or disable
  /// logic on this thread
  std::atomic<bool> _enable_external{true};

  size_t page_size;

  /// Code Caches
  app_pc cc_flush{};

  /* XCX registers */
  drvector_t allowed_xcx{};

  module::Cache mc{};

  /// fast random numbers for sampling
  std::mt19937 _prng;

  /// maximum length of period
  unsigned _min_period = 1;
  /// minimum length of period
  unsigned _max_period = 1;
  /// current pos in period
  int _sample_pos = 0;
  /// application statistics instance
  std::shared_ptr<Statistics> _stats;
  /// mutex to guard accesses to TLS
  DrLock _tls_rw_mutex;

  static const std::mt19937::result_type _max_value = decltype(_prng)::max();

 public:
  MemoryTracker(const std::shared_ptr<Statistics> &stats);
  ~MemoryTracker();

  static void process_buffer(void);
  static void clear_buffer(void);
  static void analyze_access(ShadowThreadState &data);
  static void flush_all_threads(ShadowThreadState &data, bool self = true,
                                bool flush_external = false);

  // Events
  void event_thread_init(void *drcontext, ShadowThreadState &data);

  void event_thread_exit(void *drcontext, ShadowThreadState &data);

  /** We transform string loops into regular loops so we can more easily
   * monitor every memory reference they make.
   */
  dr_emit_flags_t event_bb_app2app(void *drcontext, void *tag, instrlist_t *bb,
                                   bool for_trace, bool translating);

  dr_emit_flags_t event_app_analysis(void *drcontext, void *tag,
                                     instrlist_t *bb, bool for_trace,
                                     bool translating, OUT void **user_data);
  /// Instrument application instructions
  dr_emit_flags_t event_app_instruction(void *drcontext, void *tag,
                                        instrlist_t *bb, instr_t *instr,
                                        bool for_trace, bool translating,
                                        void *user_data);

  /// Returns true if this reference should be sampled
  inline bool sample_ref(ShadowThreadState &data) {
    --data.sampling_pos;
    if (params.sampling_rate == 1) return true;
    if (data.sampling_pos == 0) {
      data.sampling_pos = std::uniform_int_distribution<unsigned>{
          memory_tracker->_min_period,
          memory_tracker->_max_period}(memory_tracker->_prng);
      return true;
    }
    return false;
  }

  /**
   * \brief sample the basic block specified by tag
   *
   * As the instrumentation has to be idempotent per BB, we sample based on it's
   * address. To get an approximately uniform distribution, we shift the address
   * by the width of a pointer (log2).
   *
   * \return true if bb should be sampled (considered)
   */
  inline bool sample_bb(void *tag) const {
#ifdef COMPILE_X86
    constexpr short shift = 2;
#else
    constexpr short shift = 3;
#endif
    if (params.instr_rate != 0 &&
        ((uintptr_t)(tag) >> shift) % params.instr_rate == 0) {
      return true;
    }
    return false;
  }

  /// Sets the detector state based on the sampling condition
  inline void switch_sampling(ShadowThreadState &data) {
    // we use the highest available bit to denote if
    // we sample (analyze) this fsection or not
    static constexpr int flag_bit = sizeof(uintptr_t) * 8 - 1;
    if (!sample_ref(data)) {
      data.enabled = false;
      // set the flag
      data.event_cnt |= ((uintptr_t)1 << flag_bit);
    } else {
      // clear the flag
      data.event_cnt &= ~((uintptr_t)1 << flag_bit);
      if (!data.event_cnt) data.enabled = true;
    }
  }

  /// enable the detector (does not affect sampling)
  static inline void enable(ShadowThreadState &data) { data.enabled = true; }

  /// disable the detector (does not affect sampling)
  static inline void disable(ShadowThreadState &data) { data.enabled = false; }

  /// enable the detector during this scope
  static inline void enable_scope(ShadowThreadState &data) {
    if (--data.event_cnt <= 0) {
      enable(data);
    }
  }

  /// disable the detector during this scope
  static inline void disable_scope(ShadowThreadState &data) {
    disable(data);
    data.event_cnt++;
  }

  /**
   * \brief Update the code cache and remove items where the instrumentation
   * should change.
   *
   * We only consider traces, as other parts are not performance critical
   */
  static void update_cache(ShadowThreadState &data);

  static bool pc_in_freq(ShadowThreadState &data, void *bb);

 private:
  void code_cache_init(void);
  void code_cache_exit(void);

  // Instrumentation
  /// Inserts a jump to clean call if a flush is pending
  void insert_jmp_on_flush(void *drcontext, instrlist_t *ilist, instr_t *where,
                           reg_id_t regxcx, reg_id_t regtls,
                           instr_t *call_flush);

  /// Instrument all memory accessing instructions
  void instrument_mem_full(void *drcontext, instrlist_t *ilist, instr_t *where,
                           opnd_t ref, bool write);

  /// Instrument all memory accessing instructions (fast-mode)
  void instrument_mem_fast(void *drcontext, instrlist_t *ilist, instr_t *where,
                           opnd_t ref, bool write);

  /**
   * \brief instrument_mem is called whenever a memory reference is identified.
   *
   * It inserts code before the memory reference to to fill the memory buffer
   * and jump to our own code cache to call the clean_call when the buffer is
   * full.
   */
  inline void instrument_mem(void *drcontext, instrlist_t *ilist,
                             instr_t *where, opnd_t ref, bool write) {
    instrument_mem_fast(drcontext, ilist, where, ref, write);
  }

  /// Read data from external CB and modify instrumentation / detection
  /// accordingly
  void handle_ext_state(ShadowThreadState &data);

  void update_sampling();
};

static inline dr_emit_flags_t instr_event_bb_app2app(void *drcontext, void *tag,
                                                     instrlist_t *bb,
                                                     bool for_trace,
                                                     bool translating) {
  return memory_tracker->event_bb_app2app(drcontext, tag, bb, for_trace,
                                          translating);
}

static inline dr_emit_flags_t instr_event_app_analysis(
    void *drcontext, void *tag, instrlist_t *bb, bool for_trace,
    bool translating, OUT void **user_data) {
  return memory_tracker->event_app_analysis(drcontext, tag, bb, for_trace,
                                            translating, user_data);
}

/**
 * \brief add the inline assembly instrumentation
 *
 * \note various limitations apply in what is allowed and what not.
 * For instance, the function has to be idempotent and behave similar for
 * both translation and instr. phase.
 */
static inline dr_emit_flags_t instr_event_app_instruction(
    void *drcontext, void *tag, instrlist_t *bb, instr_t *instr, bool for_trace,
    bool translating, void *user_data) {
  return memory_tracker->event_app_instruction(
      drcontext, tag, bb, instr, for_trace, translating, user_data);
}

}  // namespace drace
