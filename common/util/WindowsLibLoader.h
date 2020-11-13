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

#include <windows.h>

#include "LibraryLoader.h"

namespace util {
class WindowsLibLoader : public LibraryLoader {
  HMODULE _lib = NULL;

 public:
  WindowsLibLoader() = default;

  /// LibraryLoader specialization for windows dlls
  explicit WindowsLibLoader(const char* filename) { load(filename); }

  ~WindowsLibLoader() { unload(); }

  /**
   * load a module
   * \return true if module was loaded
   */
  bool load(const char* filename) {
    if (loaded()) return false;

    _lib = LoadLibrary(filename);
    return NULL != _lib;
  }

  /**
   * unload the loaded module
   * \return true if a module was loaded and is now unloaded.
   */
  bool unload() {
    if (loaded()) {
      FreeLibrary(_lib);
      _lib = NULL;
      return true;
    }
    return false;
  }

  /**
   * \return true if module is loaded
   */
  bool loaded() { return _lib != NULL; }

  /// Get a pointer to the requested function in this module
  ProcedurePtr operator[](const char* proc_name) const {
    return ProcedurePtr(
        reinterpret_cast<void*>(GetProcAddress(_lib, proc_name)));
  }
};
}  // namespace util
