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

/// Flags describing characteristics of a module
enum MOD_TYPE_FLAGS : uint8_t {
  /// no information available
  UNKNOWN = 0x0,
  /// native module
  NATIVE = 0x1,
  /// managed module
  MANAGED = 0x2,
  /// this (managed module) contains sync mechanisms
  SYNC = 0x4 | MANAGED
};

}  // namespace module
}  // namespace drace
