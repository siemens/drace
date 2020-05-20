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

#include "../DrFile.h"
#include "../race-collector.h"
#include "sink.h"

#include <sstream>

namespace drace {
/// data-race exporter
namespace sink {
/// A Race exporter which creates human readable output
class HRText : public Sink {
 public:
  using self_t = HRText;

 private:
  std::shared_ptr<DrFile> _target;

 public:
  HRText() = delete;
  HRText(const self_t &) = delete;
  explicit HRText(self_t &&) = default;

  explicit HRText(std::shared_ptr<DrFile> target) : _target(target) {}

  virtual void process_single_race(const race::DecoratedRace &race) override {
    file_t handle = _target->get();
    dr_fprintf(handle, "----- DATA Race at %8lld ms runtime -----\n",
               race.elapsed.count());
    if (!race.resolved_addr.mod_name.empty()) {
      dr_fprintf(handle, "Racy-address in module: %s",
                 race.resolved_addr.get_pretty().c_str());
    }

    for (int i = 0; i != 2; ++i) {
      race::ResolvedAccess ac = (i == 0) ? race.first : race.second;

      dr_fprintf(handle, "Access %i tid: %i %s to/from %p with size %i\n", i,
                 ac.thread_id, (ac.write ? "write" : "read"),
                 (void *)ac.accessed_memory, ac.access_size);

      if (ac.onheap) {
        dr_fprintf(handle, "Block begin at %p, size %u\n", ac.heap_block_begin,
                   ac.heap_block_size);
      }
      dr_fprintf(handle, "Callstack (size %i)\n", ac.stack_size);
      if (race.is_resolved) {
        // stack is stored in reverse order, hence print inverted
        int ssize = static_cast<int>(ac.resolved_stack.size());
        for (int p = 0; p < ssize; ++p) {
          std::string pretty(ac.resolved_stack[ssize - 1 - p].get_pretty());
          dr_fprintf(handle, "%#04x PC %s", (int)p, pretty.c_str());
        }
      } else {
        dr_fprintf(handle, "-> (unresolved stack size: %u)\n", ac.stack_size);
      }
    }
    dr_fprintf(handle, "--------------------------------------------\n");
    dr_flush_file(handle);
  }

  virtual void process_all(
      const std::vector<race::DecoratedRace> &races) override {
    for (auto &r : races) {
      process_single_race(r);
    }
  }
};

}  // namespace sink
}  // namespace drace
