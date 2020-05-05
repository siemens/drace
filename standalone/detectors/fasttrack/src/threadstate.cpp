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
#include "threadstate.h"

ThreadState::ThreadState(VectorClock::TID own_tid,
                         const std::shared_ptr<ThreadState>& parent)
    : id(VectorClock::make_id(own_tid)) {
  vc.insert({own_tid, id});
  if (parent != nullptr) {
    // if parent exists vector clock
    vc = parent->vc;
  }
}

void ThreadState::inc_vc() {
  id++;  // as the lower 32 bits are clock just increase it by one
  vc[VectorClock::make_tid(id)] = id;
}
