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
#include <dr_api.h>

#include <util/LibraryLoader.h>

namespace drace {
namespace util {

/// LibraryLoader specialization to be used inside a DR client
class DrModuleLoader : public ::util::LibraryLoader {
 public:
  explicit DrModuleLoader() = default;

  explicit DrModuleLoader(const char* filename) { _load(filename); }

  ~DrModuleLoader() final { _unload(); }

  bool load(const char* filename) final { return _load(filename); }

  bool unload() final { return _unload(); }

  bool loaded() final { return _lib != nullptr; }

  ::util::ProcedurePtr operator[](const char* proc_name) const final {
    return ::util::ProcedurePtr(reinterpret_cast<void*>(
        dr_lookup_aux_library_routine(_lib, proc_name)));
  }

 private:
  /**
   * non-virtual function, as this is called
   * during construction
   */
  bool _load(const char* filename) {
    if (_lib == nullptr) {
      _lib = dr_load_aux_library(filename, NULL, NULL);
      return _lib != nullptr;
    }
    return false;
  }

  /**
   * non-virtual function, as this is called
   * during destruction
   */
  bool _unload() {
    if (_lib != nullptr) {
      dr_unload_aux_library(_lib);
      _lib = nullptr;
      return true;
    }
    return false;
  }

 private:
  dr_auxlib_handle_t _lib = nullptr;
};
}  // namespace util
}  // namespace drace
