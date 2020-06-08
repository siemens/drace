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
    : id(VectorClock::make_id(own_tid)),
      m_own_tid(own_tid) {
  vc.insert({make_th_num(id), id});
  if (parent != nullptr) {
    // if parent exists vector clock
    vc = parent->vc;
  }
}

void ThreadState::inc_vc() {
  id++;  // as the lower 18 bits are clock just increase it by one
  vc[make_th_num(id)] = id;
}
