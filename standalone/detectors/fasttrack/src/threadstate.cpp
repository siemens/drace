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

#include "threadstate.h"

ThreadState::ThreadState(VectorClock::TID own_tid,
                         const std::shared_ptr<ThreadState>& parent)
    : id(VectorClock::make_epoch(own_tid)), own_tid(own_tid) {
  vc.insert({make_thread_num(id), id});
  if (parent != nullptr) {
    // if parent exists vector clock
    vc = parent->vc;
  }
}

void ThreadState::inc_vc() {
  id++;  // as the lower 18 bits are clock just increase it by one
  vc[make_thread_num(id)] = id;
}

void ThreadState::set_read_write(std::size_t addr, std::size_t pc) {
  auto it = read_write.find(addr);
  if (it == read_write.end()) {
    read_write.insert({addr, {pc, traceDepot.get_current_element()}});
  } else {
    it->second = {pc, traceDepot.get_current_element()};
  }
}

std::deque<size_t> ThreadState::return_stack_trace(std::size_t address) const {
  auto it = read_write.find(address);
  if (it != read_write.end()) {
    auto data = it->second;
    return traceDepot.make_trace(data);
  }
  // A read/write operation was not tracked correctly => return empty
  // stacktrace
  return {};
}
