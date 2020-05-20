#pragma once
/*
 * DRace, a dynamic data race detector
 *
 * Copyright 2020 Siemens AG
 *
 * Authors:
 *   Felix Moessbauer <felix.moessbauer@siemens.com>
 *
 * SPDX-License-Identifier: MIT
 */

namespace drace {
namespace module {

/// Instrumentation Level Flags
enum INSTR_FLAGS : uint8_t {
  /// do not instrument this module
  NONE = 0,
  /// instrument functions (e.g. exclude)
  SYMBOLS = 1,
  /// instrument memory accesses
  MEMORY = 2,
  /// add shadow-stack instrumentation
  STACK = 4
};

}  // namespace module
}  // namespace drace
