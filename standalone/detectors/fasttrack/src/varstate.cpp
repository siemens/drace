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
#include "varstate.h"

/// evaluates for write/write races through this and and access through t
bool VarState::is_ww_race(ThreadState* t) const {
  if (get_write_epoch() != VAR_NOT_INIT && t->get_th_num() != get_write_thread_num() &&
      get_write_clock() >= t->get_clock_by_th_num(get_write_thread_num())) {
    return true;
  }
  return false;
}

/// evaluates for write/read races through this and and access through t
bool VarState::is_wr_race(ThreadState* t) const {
  auto var_th_num = get_write_thread_num();
  if (get_write_epoch() != VAR_NOT_INIT && (var_th_num != t->get_th_num()) &&
      (get_write_clock() >= t->get_clock_by_th_num(var_th_num))) {
    return true;
  }
  return false;
}

/// evaluates for read-exclusive/write races through this and and access through
/// t
bool VarState::is_rw_ex_race(ThreadState* t) const {
  auto var_th_num = get_read_thread_num();
  if (get_read_epoch() != VAR_NOT_INIT && t->get_th_num() != var_th_num &&
      get_read_clock() >= t->get_clock_by_th_num(var_th_num))  // read-write race
  {
    return true;
  }
  return false;
}

/// evaluates for read-shared/write races through this and and access through t
VectorClock<>::ThreadNum VarState::is_rw_sh_race(
    ThreadState* t, xvector<VectorClock<>::VC_EPOCH>* shared_vc) const {
  for (uint32_t i = 0; i < shared_vc->size(); ++i) {
    VectorClock<>::VC_EPOCH act_id = get_sh_id(i, shared_vc);
    VectorClock<>::ThreadNum act_th_num = VectorClock<>::make_thread_num(act_id);

    if (act_id != 0 && t->get_th_num() != act_th_num &&
        VectorClock<>::make_clock(act_id) >=
            t->get_clock_by_th_num(act_th_num)) {
      return act_th_num;
    }
  }
  return 0;
}

// TODO: optimize using vector instructions
std::vector<VectorClock<>::VC_EPOCH>::iterator VarState::find_in_vec(
    VectorClock<>::ThreadNum th_num, xvector<VectorClock<>::VC_EPOCH>* shared_vc)
    {
  auto it = shared_vc->begin();
  auto it_end = shared_vc->end();
  for (; it != it_end; ++it) {
    if (VectorClock<>::make_thread_num(*it) ==
        th_num) {
      return it;
    }
  }
  return shared_vc->end();
}

/// if in read_shared state, then returns thread id of position pos in vector
/// clock
VectorClock<>::VC_EPOCH VarState::get_sh_id(
    uint32_t pos, xvector<VectorClock<>::VC_EPOCH>* shared_vc) const {
  if (pos < shared_vc->size()) {
    return (*shared_vc)[pos];
  }
  return 0;
}

/// return stored clock value, which belongs to ThreadState t, 0 if not
/// available
VectorClock<>::VC_EPOCH VarState::get_vc_by_th_num(
    VectorClock<>::ThreadNum th_num,
    xvector<VectorClock<>::VC_EPOCH>* shared_vc) const {
  auto it = find_in_vec(th_num, shared_vc);
  if (it != shared_vc->end()) {
    return *it;
  }
  return 0;
}

VectorClock<>::Clock VarState::get_clock_by_th_num(
    VectorClock<>::ThreadNum th_num,
    xvector<VectorClock<>::VC_EPOCH>* shared_vc) const {
  auto it = find_in_vec(th_num, shared_vc);
  if (it != shared_vc->end()) {
    return VectorClock<>::make_clock(*it);
  }
  return 0;
}
