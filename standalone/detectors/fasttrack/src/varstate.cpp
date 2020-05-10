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
bool VarState::is_ww_race(ThreadState* t) const
{
  if (get_write_id() != VAR_NOT_INIT && t->get_tid() != get_w_tid() &&
      get_w_clock() >= t->get_clock_by_tid(get_w_tid())) {
    return true;
  }
  return false;
}

/// evaluates for write/read races through this and and access through t
bool VarState::is_wr_race(ThreadState* t) const
{
  auto var_tid = get_w_tid();
  if (get_write_id() != VAR_NOT_INIT && var_tid != t->get_tid() &&
      get_w_clock() >= t->get_clock_by_tid(var_tid)) {
    return true;
  }
  return false;
}

/// evaluates for read-exclusive/write races through this and and access through
/// t
bool VarState::is_rw_ex_race(ThreadState* t) const
{
  auto var_tid = get_r_tid();
  if (get_read_id() != VAR_NOT_INIT && t->get_tid() != var_tid &&
      get_r_clock() >= t->get_clock_by_tid(var_tid))  // read-write race
  {
    return true;
  }
  return false;
}

/// evaluates for read-shared/write races through this and and access through t
VectorClock<>::TID VarState::is_rw_sh_race(ThreadState* t, xvector<std::size_t>* shared_vc) const
{
  for (unsigned int i = 0; i < shared_vc->size(); ++i) {
    VectorClock<>::VC_ID act_id = get_sh_id(i, shared_vc);
    VectorClock<>::TID act_tid = VectorClock<>::make_tid(act_id);

    if (act_id != 0 && t->get_tid() != act_tid &&
        VectorClock<>::make_clock(act_id) >= t->get_clock_by_tid(act_tid)) {
      return act_tid;
    }
  }
  return 0;
}

// TODO: optimize using vector instructions
std::vector<std::size_t>::iterator VarState::find_in_vec(VectorClock<>::TID tid, xvector<std::size_t>* shared_vc) const
{ //TODO: make it public & add the shared_vc argument
  for (auto it = shared_vc->begin(); it != shared_vc->end(); ++it) {
    if (VectorClock<>::make_tid(*it) == tid) {
      return it;
    }
  }
  return shared_vc->end();
}

/**
 * \brief updates the var state because of an new read or write access through
 * an thread \todo check thread-safety
 */
//void VarState::update(bool is_write, VectorClock<>::VC_ID id) {
//  if (is_write)
//  {// we have to do shared_vcs.erase() here
//    shared_vc = nullptr; //.reset();
//    r_id = VAR_NOT_INIT;
//    w_id = id;
//    return;
//  }
//
//  if (shared_vc == nullptr) {
//    r_id = id;
//    return;
//  }
//
//  auto it = find_in_vec(VectorClock<>::make_tid(id));
//  if (it != shared_vc->end()) {
//    shared_vc->erase(it);
//  }
//  shared_vc->push_back(id);
//}
//
///// sets read state to shared
//void VarState::set_read_shared(VectorClock<>::VC_ID id)
//{ // no need for find here, but we have to access hashmap shared_vcs
//  shared_vc = std::make_unique<xvector<size_t>>();
//  shared_vc->reserve(2);
//  shared_vc->push_back(r_id);
//  shared_vc->push_back(id);
//
//  r_id = VAR_NOT_INIT;
//}

/// if in read_shared state, then returns thread id of position pos in vector clock
VectorClock<>::VC_ID VarState::get_sh_id(uint32_t pos, xvector<std::size_t>* shared_vc) const {
  if (pos < shared_vc->size()) {
    return (*shared_vc)[pos];
  }
  return 0;
}

/// return stored clock value, which belongs to ThreadState t, 0 if not available
VectorClock<>::VC_ID VarState::get_vc_by_thr(VectorClock<>::TID tid, xvector<std::size_t>* shared_vc) const {
  auto it = find_in_vec(tid, shared_vc);
  if (it != shared_vc->end()) {
    return *it;
  }
  return 0;
}

VectorClock<>::Clock VarState::get_clock_by_thr(VectorClock<>::TID tid, xvector<std::size_t>* shared_vc) const {
  auto it = find_in_vec(tid, shared_vc);
  if (it != shared_vc->end()) {
    return VectorClock<>::make_clock(*it);
  }
  return 0;
}
