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

#include <detector/Detector.h>
#include <chrono>
#include "ResolvedAccess.h"

namespace drace {
namespace race {
/**
 * \brief Single data-race with two access entries
 */
class DecoratedRace {
 public:
  ResolvedAccess first;
  ResolvedAccess second;
  symbol::SymbolLocation resolved_addr;
  std::chrono::milliseconds elapsed;
  bool is_resolved{false};

  DecoratedRace(const Detector::Race& r,
                /// elapsed time since program start
                const std::chrono::milliseconds& ttr)
      : first(r.first), second(r.second), elapsed(ttr) {}

  DecoratedRace(const ResolvedAccess& a, const ResolvedAccess& b,
                /// elapsed time since program start
                const std::chrono::milliseconds& ttr)
      : first(a), second(b), elapsed(ttr), is_resolved(true) {}
};
}  // namespace race
}  // namespace drace
