#ifndef FASTTRACK_WRAPPER_H
#define FASTTRACK_WRAPPER_H

/*
 * DRace, a dynamic data race detector
 *
 * Copyright 2020 Siemens AG
 *
 * Authors:
 *   Mihai Robescu <mihai-gabriel.robescu@siemens.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "fasttrack.h"

namespace drace {
namespace detector {

template <class LockT>
class FasttrackWrapper : public Fasttrack<LockT> {
 public:
  explicit FasttrackWrapper(bool log = true) { log_flag = log; };

  log_counters& getData() { return log_count; }

  void clear_var_state_helper(std::size_t addr) { clear_var_state(addr); }
};

}  // namespace detector
}  // namespace drace

#endif  // !FASTTRACK_WRAPPER_H
