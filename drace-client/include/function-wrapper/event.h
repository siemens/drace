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

#include "ShadowThreadState.h"

#include <dr_api.h>
#include <drmgr.h>

namespace drace {

namespace funwrap {

/// Provides callbacks for certain events, triggered by DRace
class event {
#ifdef WINDOWS
  /// Arguments of a WaitForMultipleObjects call
  struct wfmo_args_t {
    DWORD ncount;
    BOOL waitall;
    const HANDLE *handles;
  };
#endif

 private:
  static void prepare_and_aquire(void *wrapctx, void *mutex, bool write,
                                 bool trylock);

  static inline void prepare_and_release(void *wrapctx, bool write);

 public:
  static void beg_excl_region(ShadowThreadState &data);
  static void end_excl_region(ShadowThreadState &data);

  static void alloc_pre(void *wrapctx, void **user_data);
  static void alloc_post(void *wrapctx, void *user_data);

  static void realloc_pre(void *wrapctx, void **user_data);
  // realloc_post = alloc_post

  static void free_pre(void *wrapctx, void **user_data);
  static void free_post(void *wrapctx, void *user_data);

  static void begin_excl(void *wrapctx, void **user_data);
  static void end_excl(void *wrapctx, void *user_data);

  static void dotnet_enter(void *wrapctx, void **user_data);
  static void dotnet_leave(void *wrapctx, void *user_data);

  static void suppr_addr(void *wrapctx, void **user_data);

  // ------------
  // Mutex Events
  // ------------

  /// Get addr of mutex and flush the mem-ref buffer
  static void get_arg(void *wrapctx, OUT void **user_data);
  static void get_arg_dotnet(void *wrapctx, OUT void **user_data);

  static void mutex_lock(void *wrapctx, void *user_data);
  static void mutex_trylock(void *wrapctx, void *user_data);
  static void mutex_unlock(void *wrapctx, OUT void **user_data);

  static void recmutex_lock(void *wrapctx, void *user_data);
  static void recmutex_trylock(void *wrapctx, void *user_data);
  static void recmutex_unlock(void *wrapctx, OUT void **user_data);

  static void mutex_read_lock(void *wrapctx, void *user_data);
  static void mutex_read_trylock(void *wrapctx, void *user_data);
  static void mutex_read_unlock(void *wrapctx, OUT void **user_data);

#ifdef WINDOWS
  /**
   * \brief WaitForSingleObject Windows API call (experimental)
   * \warning the mutex parameter is the Handle ID of the mutex and not
   *          a memory location
   * \warning the return value is a DWORD (aka 32bit unsigned integer)
   * \warning A handle does not have to be a mutex,
   *          however distinction is not possible here
   */
  static void wait_for_single_obj(void *wrapctx, void *mutex);

  static void wait_for_mo_getargs(void *wrapctx, OUT void **user_data);

  /// WaitForMultipleObjects Windows API call (experimental)
  static void wait_for_mult_obj(void *wrapctx, void *user_data);
#endif
  /// Thread start event on caller side
  static void thread_start(void *wrapctx, void *user_data);

  /// Call this function before a thread-barrier is entered */
  static void barrier_enter(void *wrapctx, void **user_data);

  /// Call this function after a thread-barrier is passed */
  static void barrier_leave(void *wrapctx, void *user_data);

  /// Call this function after a thread-barrier is passed or the waiting was
  /// cancelled
  static void barrier_leave_or_cancel(void *wrapctx, void *user_data);

  /// Custom annotated happens before
  static void happens_before(void *wrapctx, void *identifier);

  /// Custom annotated happens after
  static void happens_after(void *wrapctx, void *identifier);

  /// Default call instrumentation
  static void on_func_call(app_pc *call_ins, app_pc *target_addr);

  /// Default return instrumentation
  static void on_func_ret(app_pc *ret_ins, app_pc *target_addr);
};
}  // namespace funwrap
}  // namespace drace
