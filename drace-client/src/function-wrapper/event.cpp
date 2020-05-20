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

#include <detector/Detector.h>
#include "ShadowThreadState.h"
#include "function-wrapper.h"
#include "globals.h"
#include "memory-tracker.h"
#include "race-collector.h"
#include "statistics.h"
#include "symbols.h"
#include "util.h"

#include <dr_api.h>
#include <drmgr.h>
#include <drwrap.h>
#include <hashtable.h>
#include <cstddef>
#include <cstdint>

// on CLR applications we sometimes get no wrapctx
// this issue can be reproduced using the cs-lock application with sync = mutex.
// this happens if an abnormal stack unwind, such as longjmp or
// a Windows exception, occurs
#define SKIP_ON_EXCEPTION(wrapctx)                              \
  do {                                                          \
    if ((wrapctx) == NULL) {                                    \
      LOG_NOTICE(0, "exception occured in wrapped allocation"); \
      return;                                                   \
    }                                                           \
  } while (0)

namespace drace {
namespace funwrap {

void event::beg_excl_region(ShadowThreadState &data) {
  // We do not flush here, as in disabled state no
  // refs are recorded
  // memory_tracker->process_buffer();
  LOG_TRACE(data.tid, "Begin excluded region");
  MemoryTracker::disable_scope(data);
}

void event::end_excl_region(ShadowThreadState &data) {
  LOG_TRACE(data.tid, "End excluded region");
  MemoryTracker::enable_scope(data);
}

// TODO: On Linux size is arg 0
void event::alloc_pre(void *wrapctx, void **user_data) {
  app_pc drcontext = drwrap_get_drcontext(wrapctx);
  ShadowThreadState &data = thread_state.getSlot(drcontext);

  MemoryTracker::flush_all_threads(data);
  // Save allocate size to user_data
  // we use the pointer directly to avoid an allocation
  // ShadowThreadState * data =
  // (ShadowThreadState*)drmgr_get_tls_field(drcontext, MemoryTracker::tls_idx);
  *user_data = drwrap_get_arg(wrapctx, 2);
}

void event::alloc_post(void *wrapctx, void *user_data) {
  SKIP_ON_EXCEPTION(wrapctx);

  app_pc drcontext = drwrap_get_drcontext(wrapctx);
  void *retval = drwrap_get_retval(wrapctx);
  void *pc = drwrap_get_func(wrapctx);
  size_t size = reinterpret_cast<size_t>(user_data);

  ShadowThreadState &data = thread_state.getSlot(drcontext);

  // allocations with size 0 are valid if they come from
  // reallocate (in fact, that's a free)
  if (size != 0) {
    // TODO: optimize tsan wrapper internally
    detector->allocate(data.detector_data, pc, retval, size);
  }
}

void event::realloc_pre(void *wrapctx, void **user_data) {
  app_pc drcontext = drwrap_get_drcontext(wrapctx);
  ShadowThreadState &data = thread_state.getSlot(drcontext);

  MemoryTracker::flush_all_threads(data);

  // first deallocate, then allocate again
  void *old_addr = drwrap_get_arg(wrapctx, 2);
  detector->deallocate(data.detector_data, old_addr);

  *user_data = drwrap_get_arg(wrapctx, 3);
  // LOG_INFO(data.tid, "reallocate, new blocksize %u at %p",
  // (SIZE_T)*user_data, old_addr);
}

// TODO: On Linux addr is arg 0
void event::free_pre(void *wrapctx, void **user_data) {
  app_pc drcontext = drwrap_get_drcontext(wrapctx);
  ShadowThreadState &data = thread_state.getSlot(drcontext);

  MemoryTracker::flush_all_threads(data);

  void *addr = drwrap_get_arg(wrapctx, 2);
  detector->deallocate(data.detector_data, addr);
}

void event::free_post(void *wrapctx, void *user_data) {
  // app_pc drcontext = drwrap_get_drcontext(wrapctx);
  // ShadowThreadState &data = thread_state.getSlot(drcontext);

  // end_excl_region(data);
  // dr_fprintf(STDERR, "<< [%i] post free\n", data.tid);
}

void event::begin_excl(void *wrapctx, void **user_data) {
  app_pc drcontext = drwrap_get_drcontext(wrapctx);
  ShadowThreadState &data = thread_state.getSlot(drcontext);
  beg_excl_region(data);
}

void event::end_excl(void *wrapctx, void *user_data) {
  SKIP_ON_EXCEPTION(wrapctx);
  app_pc drcontext = drwrap_get_drcontext(wrapctx);
  ShadowThreadState &data = thread_state.getSlot(drcontext);
  end_excl_region(data);
}

void event::dotnet_enter(void *wrapctx, void **user_data) {}
void event::dotnet_leave(void *wrapctx, void *user_data) {}

void event::suppr_addr(void *wrapctx, void **user_data) {
  void *addr = drwrap_get_arg(wrapctx, 0);
  auto &rc_inst = RaceCollector::get_instance();

  std::lock_guard<RaceCollector::MutexT> lg(rc_inst.get_mutex());
  rc_inst.get_racefilter().suppress_addr(util::unsafe_ptr_cast<uint64_t>(addr));
}

// --------------------------------------------------------------------------------------
// ------------------------------------- Mutex Events
// -----------------------------------
// --------------------------------------------------------------------------------------

void event::prepare_and_aquire(void *wrapctx, void *mutex, bool write,
                               bool trylock) {
  SKIP_ON_EXCEPTION(wrapctx);
  app_pc drcontext = drwrap_get_drcontext(wrapctx);
  ShadowThreadState &data = thread_state.getSlot(drcontext);

  if (params.exclude_master && data.tid == runtime_tid) return;

  if (trylock) {
    int retval = util::unsafe_ptr_cast<int>(drwrap_get_retval(wrapctx));
    LOG_TRACE(data.tid, "Try Aquire %p, res: %i", mutex, retval);
    // dr_printf("[%.5i] Try Aquire %p, ret %i\n", data.tid, mutex, retval);
    // If retval == 0, mtx acquired
    // skip otherwise
    if (retval != 0) return;
  }
  LOG_TRACE(data.tid, "Aquire  %p : %s", mutex,
            module_tracker->_syms->get_symbol_info(drwrap_get_func(wrapctx))
                .sym_name.c_str());

  // To avoid deadlock in flush-waiting spinlock,
  // acquire / release must not occur concurrently

  uintptr_t cnt =
      (uintptr_t)hashtable_add_replace(&data.mutex_book, mutex, (void *)1);
  if (cnt > 1) {
    hashtable_add_replace(&data.mutex_book, mutex, (void *)++cnt);
  }

  LOG_TRACE(data.tid, "Mutex book size: %i, count: %i, mutex: %p\n",
            data.mutex_book.size(), cnt, mutex);

  detector->acquire(data.detector_data, mutex, (int)cnt, write);
  // detector::happens_after(data.tid, mutex);

  data.stats.mutex_ops++;
}

void event::prepare_and_release(void *wrapctx, bool write) {
  SKIP_ON_EXCEPTION(wrapctx);
  app_pc drcontext = drwrap_get_drcontext(wrapctx);
  ShadowThreadState &data = thread_state.getSlot(drcontext);

  if (params.exclude_master && data.tid == runtime_tid) return;

  void *mutex = drwrap_get_arg(wrapctx, 0);
  // detector::happens_before(data.tid, mutex);

  uintptr_t cnt = (uintptr_t)hashtable_lookup(&data.mutex_book, mutex);
  if (cnt == 0) {
    LOG_TRACE(data.tid, "Mutex Error %p at : %s", mutex,
              module_tracker->_syms->get_symbol_info(drwrap_get_func(wrapctx))
                  .sym_name.c_str());
    // mutex not in book
    return;
  }
  if (cnt > 1) {
    hashtable_add_replace(&data.mutex_book, mutex, (void *)--cnt);
  } else if (cnt == 1) {
    hashtable_remove(&data.mutex_book, mutex);
  }

  // To avoid deadlock in flush-waiting spinlock,
  // acquire / release must not occur concurrently

  MemoryTracker::flush_all_threads(data);
  LOG_TRACE(data.tid, "Release %p : %s", mutex,
            module_tracker->_syms->get_symbol_info(drwrap_get_func(wrapctx))
                .sym_name.c_str());
  detector->release(data.detector_data, mutex, write);
}

void event::get_arg(void *wrapctx, OUT void **user_data) {
  *user_data = drwrap_get_arg(wrapctx, 0);

  app_pc drcontext = drwrap_get_drcontext(wrapctx);
  ShadowThreadState &data = thread_state.getSlot(drcontext);
  // we flush here to avoid tracking sync-function itself
  MemoryTracker::flush_all_threads(data);
}

void event::get_arg_dotnet(void *wrapctx, OUT void **user_data) {
  LOG_INFO(-1, "Hit Whatever with arg %p", drwrap_get_arg(wrapctx, 0));
  *user_data = drwrap_get_arg(wrapctx, 0);
}

void event::mutex_lock(void *wrapctx, void *user_data) {
  prepare_and_aquire(wrapctx, user_data, true, false);
}

void event::mutex_trylock(void *wrapctx, void *user_data) {
  prepare_and_aquire(wrapctx, user_data, true, true);
}

void event::mutex_unlock(void *wrapctx, OUT void **user_data) {
  prepare_and_release(wrapctx, true);
}

void event::recmutex_lock(void *wrapctx, void *user_data) {
  // TODO: Check recursive parameter
  event::prepare_and_aquire(wrapctx, user_data, true, false);
}

void event::recmutex_trylock(void *wrapctx, void *user_data) {
  prepare_and_aquire(wrapctx, user_data, true, true);
}

void event::recmutex_unlock(void *wrapctx, OUT void **user_data) {
  prepare_and_release(wrapctx, true);
}

void event::mutex_read_lock(void *wrapctx, void *user_data) {
  prepare_and_aquire(wrapctx, user_data, false, false);
}

void event::mutex_read_trylock(void *wrapctx, void *user_data) {
  prepare_and_aquire(wrapctx, user_data, false, true);
}

void event::mutex_read_unlock(void *wrapctx, OUT void **user_data) {
  prepare_and_release(wrapctx, false);
}

#ifdef WINDOWS
void event::wait_for_single_obj(void *wrapctx, void *mutex) {
  // https://docs.microsoft.com/en-us/windows/desktop/api/synchapi/nf-synchapi-waitforsingleobject

  SKIP_ON_EXCEPTION(wrapctx);

  app_pc drcontext = drwrap_get_drcontext(wrapctx);
  ShadowThreadState &data = thread_state.getSlot(drcontext);

  LOG_TRACE(data.tid, "waitForSingleObject: %p\n", mutex);

  if (params.exclude_master && data.tid == runtime_tid) return;

  DWORD retval = util::unsafe_ptr_cast<DWORD>(drwrap_get_retval(wrapctx));
  if (retval != WAIT_OBJECT_0) return;

  LOG_TRACE(data.tid, "waitForSingleObject: %p (Success)", mutex);

  // add or increment counter in table
  uintptr_t cnt =
      (uintptr_t)hashtable_add_replace(&data.mutex_book, mutex, (void *)1);
  if (cnt > 1) {
    hashtable_add_replace(&data.mutex_book, mutex, (void *)++cnt);
  }

  MemoryTracker::flush_all_threads(data);
  detector->acquire(data.detector_data, mutex, (int)cnt, 1);
  data.stats.mutex_ops++;
}

void event::wait_for_mo_getargs(void *wrapctx, OUT void **user_data) {
  app_pc drcontext = drwrap_get_drcontext(wrapctx);
  ShadowThreadState &data = thread_state.getSlot(drcontext);

  wfmo_args_t *args =
      (wfmo_args_t *)dr_thread_alloc(drcontext, sizeof(wfmo_args_t));

  args->ncount = util::unsafe_ptr_cast<DWORD>(drwrap_get_arg(wrapctx, 0));
  args->handles =
      util::unsafe_ptr_cast<const HANDLE *>(drwrap_get_arg(wrapctx, 1));
  args->waitall = util::unsafe_ptr_cast<BOOL>(drwrap_get_arg(wrapctx, 2));

  LOG_TRACE(data.tid, "waitForMultipleObjects: %u, %i", args->ncount,
            args->waitall);

  *user_data = (void *)args;
}

void event::wait_for_mult_obj(void *wrapctx, void *user_data) {
  SKIP_ON_EXCEPTION(wrapctx);

  app_pc drcontext = drwrap_get_drcontext(wrapctx);
  ShadowThreadState &data = thread_state.getSlot(drcontext);
  DWORD retval = util::unsafe_ptr_cast<DWORD>(drwrap_get_retval(wrapctx));

  wfmo_args_t *info = (wfmo_args_t *)user_data;

  MemoryTracker::flush_all_threads(data);
  if (info->waitall && (retval == WAIT_OBJECT_0)) {
    LOG_TRACE(data.tid, "waitForMultipleObjects:finished all");
    for (DWORD i = 0; i < info->ncount; ++i) {
      HANDLE mutex = info->handles[i];

      uintptr_t cnt =
          (uintptr_t)hashtable_add_replace(&data.mutex_book, mutex, (void *)1);
      if (cnt > 1) {
        hashtable_add_replace(&data.mutex_book, mutex, (void *)++cnt);
      }

      detector->acquire(data.detector_data, (void *)mutex, (int)cnt, true);
      data.stats.mutex_ops++;
    }
  }
  if (!info->waitall) {
    if (retval <= (WAIT_OBJECT_0 + info->ncount - 1)) {
      HANDLE mutex = info->handles[retval - WAIT_OBJECT_0];
      LOG_TRACE(data.tid, "waitForMultipleObjects:finished one: %p", mutex);

      uintptr_t cnt =
          (uintptr_t)hashtable_add_replace(&data.mutex_book, mutex, (void *)1);
      if (cnt > 1) {
        hashtable_add_replace(&data.mutex_book, mutex, (void *)++cnt);
      }

      detector->acquire(data.detector_data, (void *)mutex, (int)cnt, true);
      data.stats.mutex_ops++;
    }
  }

  dr_thread_free(drcontext, user_data, sizeof(wfmo_args_t));
}
#endif  // WINDOWS

void event::thread_start(void *wrapctx, void *user_data) {
#ifdef WINDOWS
  app_pc drcontext = drwrap_get_drcontext(wrapctx);
  ShadowThreadState &data = thread_state.getSlot(drcontext);
  HANDLE retval = util::unsafe_ptr_cast<HANDLE>(drwrap_get_retval(wrapctx));
  // the return value contains a handle to the thread, but we need the unique id
  DWORD threadid = GetThreadId(retval);
  LOG_TRACE(data.tid, "Thread started with handle: %d, ID: %d", retval,
            threadid);
  detector->happens_before(data.detector_data, (void *)(uintptr_t)threadid);
#else
// \todo implement on linux
#endif
}

void event::barrier_enter(void *wrapctx, void **addr) {
  app_pc drcontext = drwrap_get_drcontext(wrapctx);
  ShadowThreadState &data = thread_state.getSlot(drcontext);
  *addr = drwrap_get_arg(wrapctx, 0);
  LOG_TRACE(static_cast<detector::tid_t>(data.tid), "barrier enter %p", *addr);
  // each thread enters the barrier individually

  detector->happens_before(data.detector_data, *addr);
}

void event::barrier_leave(void *wrapctx, void *addr) {
  SKIP_ON_EXCEPTION(wrapctx);

  app_pc drcontext = drwrap_get_drcontext(wrapctx);
  ShadowThreadState &data = thread_state.getSlot(drcontext);

  LOG_TRACE(data.tid, "barrier passed");

  // each thread leaves individually, but only after all barrier_enters have
  // been called
  detector->happens_after(data.detector_data, addr);
}

void event::barrier_leave_or_cancel(void *wrapctx, void *addr) {
  SKIP_ON_EXCEPTION(wrapctx);
  app_pc drcontext = drwrap_get_drcontext(wrapctx);
  ShadowThreadState &data = thread_state.getSlot(drcontext);

  bool passed = (bool)drwrap_get_retval(wrapctx);
  LOG_TRACE(data.tid, "barrier left with status %i", passed);

  // TODO: Validate cancellation path, where happens_before will be called again
  if (passed) {
    // each thread leaves individually, but only after all barrier_enters have
    // been called
    detector->happens_after(data.detector_data, addr);
  }
}

void event::happens_before(void *wrapctx, void *identifier) {
  SKIP_ON_EXCEPTION(wrapctx);

  app_pc drcontext = drwrap_get_drcontext(wrapctx);
  ShadowThreadState &data = thread_state.getSlot(drcontext);
  detector->happens_before(data.detector_data, identifier);
  LOG_TRACE(data.tid, "happens-before @ %p", identifier);
}

void event::happens_after(void *wrapctx, void *identifier) {
  SKIP_ON_EXCEPTION(wrapctx);

  app_pc drcontext = drwrap_get_drcontext(wrapctx);
  ShadowThreadState &data = thread_state.getSlot(drcontext);
  detector->happens_after(data.detector_data, identifier);
  LOG_TRACE(data.tid, "happens-after  @ %p", identifier);
}

/// Default call instrumentation
void event::on_func_call(app_pc *call_ins, app_pc *target_addr) {
  ShadowThreadState &data = thread_state.getSlot();

  memory_tracker->analyze_access(data);
  // Sampling: Possibly disable detector during this function
  memory_tracker->switch_sampling(data);

  // if lossy_flush, disable detector instead of changing the instructions
  if (params.lossy && !params.lossy_flush &&
      MemoryTracker::pc_in_freq(data, call_ins)) {
    data.enabled = false;
  }

  data.stack.push(call_ins, data.detector_data);
}

/// Default return instrumentation
void event::on_func_ret(app_pc *ret_ins, app_pc *target_addr) {
  ShadowThreadState &data = thread_state.getSlot();
  ShadowStack &stack = data.stack;
  MemoryTracker::analyze_access(data);

  if (stack.isEmpty()) return;

  ptrdiff_t diff;
  // leave this scope / call
  while ((diff = (byte *)target_addr - (byte *)stack.pop(data.detector_data)),
         !(0 <= diff && diff <= sizeof(void *))) {
    // skipping a frame
    if (stack.isEmpty()) return;
  }
}
}  // namespace funwrap
}  // namespace drace
