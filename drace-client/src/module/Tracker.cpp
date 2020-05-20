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
#include "globals.h"

#include "function-wrapper.h"
#include "memory-tracker.h"
#include "statistics.h"
#include "symbol/Symbols.h"
#include "util.h"

#ifdef WINDOWS
#include "ipc/SMData.h"
#include "ipc/SharedMemory.h"
#include "ipc/SyncSHMDriver.h"
#endif
#include "MSR.h"

#include <drmgr.h>
#include <drutil.h>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include <regex>

namespace drace {
namespace module {

Tracker::Tracker(const std::shared_ptr<symbol::Symbols> &symbols)
    : _syms(symbols) {
  mod_lock = dr_rwlock_create();

  excluded_mods = config.get_multi("modules", "exclude_mods");
  excluded_path_prefix = config.get_multi("modules", "exclude_path");

  // convert pathes to lowercase for case-insensitive matching
  for (auto &prefix : excluded_path_prefix) {
    std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::tolower);
  }

  if (!drmgr_register_module_load_event(event_module_load) ||
      !drmgr_register_module_unload_event(event_module_unload)) {
    DR_ASSERT(false);
  }
}

Tracker::~Tracker() {
  if (!drmgr_unregister_module_load_event(event_module_load) ||
      !drmgr_unregister_module_unload_event(event_module_unload)) {
    DR_ASSERT(false);
  }

  dr_rwlock_destroy(mod_lock);
}

Tracker::PMetadata Tracker::get_module_containing(const app_pc pc) const {
  lock_read();
  auto m_it = _modules_idx.lower_bound(pc);

  if (m_it == _modules_idx.end()) {
    unlock_read();
    return nullptr;
  }

  auto mptr = m_it->second;
  unlock_read();

  if (pc < mptr->end) {
    return mptr;
  } else {
    return nullptr;
  }
}

Tracker::PMetadata Tracker::register_module(const module_data_t *mod,
                                            bool loaded) {
  using module::INSTR_FLAGS;
  using module::MOD_TYPE_FLAGS;

  // first check if module is already registered
  lock_read();
  auto modptr = get_module_containing(mod->start);
  unlock_read();

  if (modptr) {
    if (!modptr->loaded && (modptr->info == mod)) {
      modptr->loaded = true;
      return modptr;
    }
  }

  // Module is not registered (new)
  INSTR_FLAGS def_instr_flags = (INSTR_FLAGS)(
      INSTR_FLAGS::MEMORY | INSTR_FLAGS::STACK | INSTR_FLAGS::SYMBOLS);

  lock_write();
  modptr = add_emplace(mod->start, mod->end);
  unlock_write();

  // Module not already registered
  modptr->set_info(mod);
  modptr->instrument = def_instr_flags;

#ifdef WINDOWS
  if (modptr->modtype == MOD_TYPE_FLAGS::MANAGED && !shmdriver) {
    LOG_WARN(0, "managed module detected, but MSR not available");
  }
#endif

  std::string mod_path(mod->full_path);
  std::string mod_name(dr_module_preferred_name(mod));
  std::transform(mod_path.begin(), mod_path.end(), mod_path.begin(), ::tolower);

  for (auto prefix : excluded_path_prefix) {
    // check if mod path is excluded
    if (util::common_prefix(prefix, mod_path)) {
      modptr->instrument = INSTR_FLAGS::STACK;
      break;
    }
  }
  // if not in excluded path
  if (modptr->instrument != INSTR_FLAGS::STACK) {
    // check if mod name is excluded
    // in this case, we check for syms but only instrument stack
    for (auto it = excluded_mods.begin(); it != excluded_mods.end(); ++it) {
      std::regex expr(*it);
      std::smatch m;
      if (std::regex_match(mod_name, m, expr)) {
        modptr->instrument =
            (INSTR_FLAGS)(INSTR_FLAGS::SYMBOLS | INSTR_FLAGS::STACK);
        break;
      }
    }
  }
  if (modptr->instrument & INSTR_FLAGS::SYMBOLS) {
    // check if debug info is available
    // \todo unclear if drsyms is threadsafe.
    //       so better lock
    lock_write();
    modptr->debug_info = _syms->debug_info_available(mod);
    unlock_write();
  }

  return modptr;
}

/**
 * \brief Module load event implementation.
 *
 * To get clean call-stacks, we add the shadow-stack instrumentation
 * to all modules (even the excluded ones).
 * \note As this function is passed as a callback to a c API, we cannot use
 * std::bind
 *
 */
void event_module_load(void *drcontext, const module_data_t *mod, bool loaded) {
  using INSTR_FLAGS = module::Metadata::INSTR_FLAGS;

  auto start = std::chrono::system_clock::now();
  ShadowThreadState &data = thread_state.getSlot(drcontext);

  auto modptr = module_tracker->register_module(mod, loaded);
  std::string mod_name = dr_module_preferred_name(mod);

  // wrap functions
  if (util::common_prefix(mod_name, "MSVCP") ||
      util::common_prefix(mod_name, "KERNELBASE") ||
      util::common_prefix(mod_name, "libc")) {
    funwrap::wrap_mutexes(mod, true);
  } else if (util::common_prefix(mod_name, "KERNEL")) {
    funwrap::wrap_allocations(mod);
    funwrap::wrap_thread_start_sys(mod);
  }
#ifdef WINDOWS
  // \todo Dotnet is currently only supported on windows
  else if (util::common_prefix(mod_name, "clr.dll") ||
           util::common_prefix(mod_name, "coreclr.dll")) {
    if (shmdriver) {
      bool m_ok = MSR::attach(mod);

      if (m_ok) {
        MSR::request_symbols(mod);
        funwrap::wrap_sync_dotnet(mod, true);
      }
    }
  } else if (modptr->modtype == Metadata::MOD_TYPE_FLAGS::MANAGED) {
    if (shmdriver) {
      // TODO: can we delay this until the first race happens in this module?
      MSR::request_symbols(mod);
    }
    // Name of .Net modules often contains a full path
    std::string basename = util::basename(mod_name);

    if (util::common_prefix(basename, "System.Private.CoreLib.dll")) {
      if (shmdriver) funwrap::wrap_sync_dotnet(mod, false);
    } else if (util::common_prefix(basename, "System.Threading.dll")) {
      if (shmdriver) funwrap::wrap_sync_dotnet(mod, false);
    } else if (util::common_prefix(basename, "System.Console.dll")) {
      if (shmdriver) {
        funwrap::wrap_functions(
            mod, config.get_multi("dotnetexclude", "exclude"), true,
            funwrap::Method::EXTERNAL_MPCR, funwrap::event::begin_excl,
            funwrap::event::end_excl);
      }
    }
    if (util::common_prefix(basename, "System.")) {
      // TODO: This is highly experimental
      // Check impact on correctness
      LOG_NOTICE(data.tid, "Detected %s", basename.c_str());
      modptr->instrument = INSTR_FLAGS::STACK;
      if (!util::common_prefix(basename, "System.Private")) {
        modptr->modtype = (Metadata::MOD_TYPE_FLAGS)(
            modptr->modtype | Metadata::MOD_TYPE_FLAGS::SYNC);
      }
    } else {
      // Not a System DLL
      LOG_NOTICE(data.tid, "Detected %s", basename.c_str());
      modptr->instrument = INSTR_FLAGS::STACK;
      modptr->modtype = (Metadata::MOD_TYPE_FLAGS)(
          modptr->modtype | Metadata::MOD_TYPE_FLAGS::SYNC);
    }
  }
#endif
  else if (modptr->instrument != INSTR_FLAGS::NONE && params.annotations) {
    // no special handling of this module
    funwrap::wrap_excludes(mod);
    funwrap::wrap_annotations(mod);
    // This requires debug symbols, but avoids false positives during
    // C++11 thread construction and startup
    if (modptr->debug_info) {
      // funwrap::wrap_thread_start(mod);
      funwrap::wrap_mutexes(mod, false);
    }
  }

  LOG_INFO(data.tid,
           "Track module: %20s, beg : %p, end : %p, instrument : %s, debug "
           "info : %s, full path : %s",
           mod_name.c_str(), modptr->base, modptr->end,
           util::instr_flags_to_str((uint8_t)modptr->instrument).c_str(),
           modptr->debug_info ? "YES" : " NO", modptr->info->full_path);

  // Free symbol information. A later access re-creates them, so its safe to do
  // it here
  drsym_free_resources(mod->full_path);

#ifdef WINDOWS
  // free symbols on MPCR side
  if (modptr->modtype == Metadata::MOD_TYPE_FLAGS::MANAGED && shmdriver) {
    MSR::unload_symbols(mod->start);
  }
#endif

  data.stats.module_load_duration +=
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now() - start);
  data.stats.module_loads++;
}

/* Module unload event implementation. As this function is passed
 *  as a callback to a C api, we cannot use std::bind
 */
void event_module_unload(void *drcontext, const module_data_t *mod) {
  LOG_INFO(-1, "Unload module: %19s, beg : %p, end : %p, full path : %s",
           dr_module_preferred_name(mod), mod->start, mod->end, mod->full_path);

  auto modptr = module_tracker->get_module_containing(mod->start);
  if (modptr) {
    modptr->loaded = false;
  }
}
}  // namespace module
}  // namespace drace
