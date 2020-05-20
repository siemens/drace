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

#include "function-wrapper.h"
#include "InstrumentationConfig.h"
#include "MSR.h"
#include "globals.h"
#include "memory-tracker.h"

#include <functional>
#include <string>
#include <vector>

#include <detector/Detector.h>
#include <dr_api.h>
#include <drsyms.h>
#include <drutil.h>
#include <drwrap.h>

#include <unordered_map>

namespace drace {
bool funwrap::init() {
  bool state = drwrap_init();

  // performance tuning
  drwrap_set_global_flags(
      (drwrap_global_flags_t)(DRWRAP_NO_FRILLS | DRWRAP_FAST_CLEANCALLS));
  return state;
}

void funwrap::finalize() { drwrap_exit(); }

bool funwrap::wrap_functions(const module_data_t *mod,
                             const std::vector<std::string> &syms,
                             bool full_search, funwrap::Method method,
                             wrapcb_pre_t pre, wrapcb_post_t post) {
  // set to true if at least one function is wrapped
  bool wrapped_some = false;
  const auto &fullname = dr_module_preferred_name(mod);
  std::string modname = util::without_extension(fullname);

  for (const auto &name : syms) {
    LOG_NOTICE(-1, "Search for %s", name.c_str());
    if (method == Method::EXTERNAL_MPCR) {
#ifdef WINDOWS
      std::string symname = (modname + '!') + name;
      auto sr = MSR::search_symbol(mod, symname.c_str(), full_search);
      wrapped_some |= sr.size > 0;
      for (int i = 0; i < sr.size; ++i) {
        wrap_info_t info{mod, pre, post};
        internal::wrap_function_clbck(
            "<unknown>", sr.adresses[i] - (size_t)mod->start, (void *)(&info));
      }
#endif
    } else if (method == Method::DBGSYMS) {
#if WINDOWS
      // \todo not supported on linux
      wrap_info_t info{mod, pre, post};
      drsym_error_t err = drsym_search_symbols(
          mod->full_path, name.c_str(), false,
          (drsym_enumerate_cb)internal::wrap_function_clbck, (void *)&info);
      wrapped_some |= (err == DRSYM_SUCCESS);
#endif
    } else if (method == Method::EXPORTS) {
      app_pc towrap = (app_pc)dr_get_proc_address(mod->handle, name.c_str());
      if (drwrap_wrap(towrap, pre, post)) {
        LOG_INFO(0, "Wrapped %s at %p", name.c_str(), towrap);
        wrapped_some = true;
      }
    }
  }
  return wrapped_some;
}

void funwrap::wrap_allocations(const module_data_t *mod) {
  wrap_functions(mod, config.get_multi("functions", "allocators"), false,
                 Method::EXPORTS, event::alloc_pre, event::alloc_post);
  wrap_functions(mod, config.get_multi("functions", "reallocators"), false,
                 Method::EXPORTS, event::realloc_pre, event::alloc_post);
  wrap_functions(mod, config.get_multi("functions", "deallocators"), false,
                 Method::EXPORTS, event::free_pre, nullptr);
}

void funwrap::wrap_excludes(const module_data_t *mod, std::string section) {
  wrap_functions(mod, config.get_multi(section, "exclude"), false,
                 Method::DBGSYMS, event::begin_excl, event::end_excl);
}

void funwrap::wrap_mutexes(const module_data_t *mod, bool sys) {
  using namespace internal;

  if (sys) {
    // mutex lock
    wrap_functions(mod, config.get_multi("sync", "acquire_excl"), false,
                   Method::EXPORTS, event::get_arg, event::mutex_lock);
    // mutex trylock
    wrap_functions(mod, config.get_multi("sync", "acquire_excl_try"), false,
                   Method::EXPORTS, event::get_arg, event::mutex_trylock);
    // mutex unlock
    wrap_functions(mod, config.get_multi("sync", "release_excl"), false,
                   Method::EXPORTS, event::mutex_unlock, NULL);
    // rec-mutex lock
    wrap_functions(mod, config.get_multi("sync", "acquire_rec"), false,
                   Method::EXPORTS, event::get_arg, event::recmutex_lock);
    // rec-mutex trylock
    wrap_functions(mod, config.get_multi("sync", "acquire_rec_try"), false,
                   Method::EXPORTS, event::get_arg, event::recmutex_trylock);
    // rec-mutex unlock
    wrap_functions(mod, config.get_multi("sync", "release_rec"), false,
                   Method::EXPORTS, event::recmutex_unlock, NULL);
    // read-mutex lock
    wrap_functions(mod, config.get_multi("sync", "acquire_shared"), false,
                   Method::EXPORTS, event::get_arg, event::mutex_read_lock);
    // read-mutex trylock
    wrap_functions(mod, config.get_multi("sync", "acquire_shared_try"), false,
                   Method::EXPORTS, event::get_arg, event::mutex_read_trylock);
    // read-mutex unlock
    wrap_functions(mod, config.get_multi("sync", "release_shared"), false,
                   Method::EXPORTS, event::mutex_read_unlock, NULL);
#ifdef WINDOWS
    // waitForSingleObject
    wrap_functions(mod, config.get_multi("sync", "wait_for_single_object"),
                   false, Method::EXPORTS, event::get_arg,
                   event::wait_for_single_obj);
    // waitForMultipleObjects
    wrap_functions(mod, config.get_multi("sync", "wait_for_multiple_objects"),
                   false, Method::EXPORTS, event::wait_for_mo_getargs,
                   event::wait_for_mult_obj);
#else
    wrap_functions(mod, config.get_multi("linuxsync", "acquire_excl"), false,
                   Method::EXPORTS, event::get_arg, event::mutex_lock);
    // mutex trylock
    wrap_functions(mod, config.get_multi("linuxsync", "acquire_excl_try"),
                   false, Method::EXPORTS, event::get_arg,
                   event::mutex_trylock);
    // mutex unlock
    wrap_functions(mod, config.get_multi("linuxsync", "release_excl"), false,
                   Method::EXPORTS, event::mutex_unlock, NULL);
#endif

  } else {
    LOG_INFO(0, "try to wrap non-system mutexes");
    // Std mutexes
    wrap_functions(mod, config.get_multi("stdsync", "acquire_excl"), false,
                   Method::DBGSYMS, event::get_arg, event::mutex_lock);
    wrap_functions(mod, config.get_multi("stdsync", "acquire_excl_try"), false,
                   Method::DBGSYMS, event::get_arg, event::mutex_trylock);
    wrap_functions(mod, config.get_multi("stdsync", "release_excl"), false,
                   Method::DBGSYMS, event::mutex_unlock, NULL);

    // Qt Mutexes
    wrap_functions(mod, config.get_multi("qtsync", "acquire_excl"), false,
                   Method::DBGSYMS, event::get_arg, event::recmutex_lock);
    wrap_functions(mod, config.get_multi("qtsync", "acquire_excl_try"), false,
                   Method::DBGSYMS, event::get_arg, event::recmutex_trylock);
    wrap_functions(mod, config.get_multi("qtsync", "release_excl"), false,
                   Method::DBGSYMS, event::recmutex_unlock, NULL);

    wrap_functions(mod, config.get_multi("qtsync", "acquire_shared"), false,
                   Method::DBGSYMS, event::get_arg, event::mutex_read_lock);
    wrap_functions(mod, config.get_multi("qtsync", "acquire_shared_try"), false,
                   Method::DBGSYMS, event::get_arg, event::mutex_read_trylock);
  }
}

void funwrap::wrap_thread_start_sys(const module_data_t *mod) {
  wrap_functions(mod, config.get_multi("functions", "threadstart"), false,
                 Method::EXPORTS, NULL, event::thread_start);
}

void funwrap::wrap_sync_dotnet(const module_data_t *mod, bool native) {
  using namespace internal;

  // std::string modname = util::basename(dr_module_preferred_name(mod));
  // Managed IPs
  if (!native) {
    wrap_functions(mod, config.get_multi("dotnetsync_rwlock", "acquire_excl"),
                   true, Method::EXTERNAL_MPCR, event::get_arg,
                   event::mutex_lock);
    wrap_functions(
        mod, config.get_multi("dotnetsync_rwlock", "acquire_excl_try"), true,
        Method::EXTERNAL_MPCR, event::get_arg, event::mutex_trylock);
    wrap_functions(mod, config.get_multi("dotnetsync_rwlock", "acquire_shared"),
                   true, Method::EXTERNAL_MPCR, event::get_arg,
                   event::mutex_read_lock);
    wrap_functions(
        mod, config.get_multi("dotnetsync_rwlock", "acquire_shared_try"), true,
        Method::EXTERNAL_MPCR, event::get_arg, event::mutex_read_trylock);
    // upgradable locks are semantically read-locks. However it is valid to
    // upgrade it to a write lock without relinquishing the ressource
    wrap_functions(
        mod, config.get_multi("dotnetsync_rwlock", "acquire_upgrade"), true,
        Method::EXTERNAL_MPCR, event::get_arg, event::mutex_read_lock);
    wrap_functions(
        mod, config.get_multi("dotnetsync_rwlock", "acquire_upgrade_try"), true,
        Method::EXTERNAL_MPCR, event::get_arg, event::mutex_read_trylock);

    wrap_functions(mod, config.get_multi("dotnetsync_rwlock", "release_excl"),
                   true, Method::EXTERNAL_MPCR, event::mutex_unlock, NULL);
    wrap_functions(mod, config.get_multi("dotnetsync_rwlock", "release_shared"),
                   true, Method::EXTERNAL_MPCR, event::mutex_read_unlock, NULL);
    wrap_functions(mod, config.get_multi("dotnetexclude", "exclude"), true,
                   Method::EXTERNAL_MPCR, event::begin_excl, event::end_excl);

    wrap_functions(mod, config.get_multi("dotnetsync_barrier", "wait_blocking"),
                   true, Method::EXTERNAL_MPCR, event::barrier_enter,
                   event::barrier_leave);
    wrap_functions(mod,
                   config.get_multi("dotnetsync_barrier", "wait_nonblocking"),
                   true, Method::EXTERNAL_MPCR, event::barrier_enter,
                   event::barrier_leave_or_cancel);
  } else {
    // [native] JIT_MonExit
    // We also use MPCR here, as DR syms has problems finding the correct debug
    // information if multiple versions of a dll are in the cache
    LOG_NOTICE(-1, "try to wrap dotnetsync native");
    bool wrapped_dotnet = true;
    wrapped_dotnet &= wrap_functions(
        mod, config.get_multi("dotnetsync_monitor", "monitor_enter"), false,
        Method::EXTERNAL_MPCR, event::get_arg, event::mutex_lock);
    wrapped_dotnet &= wrap_functions(
        mod, config.get_multi("dotnetsync_monitor", "monitor_exit"), false,
        Method::EXTERNAL_MPCR, event::mutex_unlock, NULL);
    if (!wrapped_dotnet) {
      LOG_WARN(0,
               "no dotnet sync functions found, expect false-positives in "
               "dotnet code");
      LOG_WARN(0, "make sure symbol lookup in MSR is working correctly");
    }
  }
}

void funwrap::wrap_annotations(const module_data_t *mod) {
  LOG_NOTICE(0, "try to wrap annotations");
  // wrap happens before
  wrap_functions(mod, config.get_multi("sync", "happens_before"), false,
                 Method::EXPORTS, event::get_arg, event::happens_before);
  wrap_functions(mod, config.get_multi("sync", "happens_after"), false,
                 Method::EXPORTS, event::get_arg, event::happens_after);

  // wrap excludes
  wrap_functions(mod, config.get_multi("functions", "exclude_enter"), false,
                 Method::EXPORTS, event::begin_excl, NULL);
  wrap_functions(mod, config.get_multi("functions", "exclude_leave"), false,
                 Method::EXPORTS, NULL, event::end_excl);
  // wrap supressions
  wrap_functions(mod, config.get_multi("functions", "exclude_argaddr"), false,
                 Method::EXPORTS, event::suppr_addr, NULL);
}

bool funwrap::wrap_generic_call(void *drcontext, void *tag, instrlist_t *bb,
                                instr_t *instr, bool for_trace,
                                bool translating, void *user_data) {
  // \todo The shadow stack instrumentation triggers many assertions
  // when running in debug mode on a CoreCLR application

  // \todo Handle dotnet calls with push addr; ret;

  if (instr == instrlist_last(bb)) {
    if (instr_is_call_direct(instr)) {
      dr_insert_call_instrumentation(drcontext, bb, instr,
                                     (void *)event::on_func_call);
      return true;
    } else if (instr_is_call_indirect(instr)) {
      dr_insert_mbr_instrumentation(drcontext, bb, instr,
                                    (void *)event::on_func_call, SPILL_SLOT_1);
      return true;
    } else if (instr_is_return(instr)) {
      dr_insert_mbr_instrumentation(drcontext, bb, instr,
                                    (void *)event::on_func_ret, SPILL_SLOT_1);
      return true;
    }
  }
  return false;
}

}  // namespace drace
