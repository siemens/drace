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
#include "../globals.h"
#include "InstrFlags.h"
#include "ModTypeFlags.h"

namespace drace {
namespace module {
/// Encapsulates and enriches a dynamorio module_data_t struct
class Metadata {
 public:
  using INSTR_FLAGS = drace::module::INSTR_FLAGS;
  using MOD_TYPE_FLAGS = drace::module::MOD_TYPE_FLAGS;

  app_pc base;
  app_pc end;
  bool loaded;
  INSTR_FLAGS instrument;
  MOD_TYPE_FLAGS modtype{MOD_TYPE_FLAGS::UNKNOWN};
  module_data_t *info{nullptr};
  bool debug_info;

 private:
  /**
   * \brief Determines if the module is a managed-code module using the PE
   * header
   */
  void tag_module();

 public:
  Metadata(app_pc mbase, app_pc mend, bool mloaded = true,
           bool debug_info = false)
      : base(mbase),
        end(mend),
        loaded(mloaded),
        instrument(INSTR_FLAGS::NONE),
        debug_info(debug_info) {}

  ~Metadata() {
    if (info != nullptr) {
      dr_free_module_data(info);
      info = nullptr;
    }
  }

  /// Copy constructor, duplicates dr module data
  Metadata(const Metadata &other)
      : base(other.base),
        end(other.end),
        loaded(other.loaded),
        instrument(other.instrument),
        modtype(other.modtype),
        debug_info(other.debug_info) {
    info = dr_copy_module_data(other.info);
  }

  /// Move constructor, moves dr module data
  Metadata(Metadata &&other)
      : base(other.base),
        end(other.end),
        loaded(other.loaded),
        instrument(other.instrument),
        info(other.info),
        modtype(other.modtype),
        debug_info(other.debug_info) {
    other.info = nullptr;
  }

  void set_info(const module_data_t *mod) {
    info = dr_copy_module_data(mod);
    tag_module();
  }

  inline Metadata &operator=(const Metadata &other) {
    if (this == &other) return *this;
    dr_free_module_data(info);
    base = other.base;
    end = other.end;
    loaded = other.loaded;
    instrument = other.instrument;
    info = dr_copy_module_data(other.info);
    modtype = other.modtype;
    debug_info = other.debug_info;
    return *this;
  }

  inline Metadata &operator=(Metadata &&other) {
    if (this == &other) return *this;
    dr_free_module_data(info);
    base = other.base;
    end = other.end;
    loaded = other.loaded;
    instrument = other.instrument;
    info = other.info;
    modtype = other.modtype;
    debug_info = other.debug_info;

    other.info = nullptr;
    return *this;
  }

  inline bool operator==(const Metadata &other) const {
    return (base == other.base);
  }
  inline bool operator!=(const Metadata &other) const {
    return !(base == other.base);
  }
  inline bool operator<(const Metadata &other) const {
    return (base < other.base);
  }
  inline bool operator>(const Metadata &other) const {
    return (base > other.base);
  }
};
}  // namespace module
}  // namespace drace
