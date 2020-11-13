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

#include <vector>

#include <detector/Detector.h>
#include "symbols.h"

namespace drace {
namespace race {
/**
 * \brief Race access entry with symbolized callstack information
 */
class ResolvedAccess : public Detector::AccessEntry {
 public:
  std::vector<symbol::SymbolLocation> resolved_stack;

  explicit ResolvedAccess(const Detector::AccessEntry& e)
      : Detector::AccessEntry(e) {}
};
}  // namespace race
}  // namespace drace
