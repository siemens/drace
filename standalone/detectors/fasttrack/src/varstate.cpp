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
  if (get_write_id() != VAR_NOT_INIT && t->get_th_num() != get_w_th_num() &&
    get_w_clock() >= t->get_clock_by_th_num(get_w_th_num())) {
    return true;
  }
  return false;
}

/// evaluates for write/read races through this and and access through t
bool VarState::is_wr_race(ThreadState* t) const {
  auto var_th_num = get_w_th_num();
  if (get_write_id() != VAR_NOT_INIT && var_th_num != t->get_th_num() &&
    get_w_clock() >= t->get_clock_by_th_num(var_th_num)) {
    return true;
  }
  return false;
}

/// evaluates for read-exclusive/write races through this and and access through
/// t
bool VarState::is_rw_ex_race(ThreadState* t) const {
  auto var_th_num = get_r_th_num();
  if (get_read_id() != VAR_NOT_INIT && t->get_th_num() != var_th_num &&
    get_r_clock() >= t->get_clock_by_th_num(var_th_num))  // read-write race
  {
    return true;
  }
  return false;
}

/// evaluates for read-shared/write races through this and and access through t
VectorClock<>::Thread_Num VarState::is_rw_sh_race(ThreadState* t, xvector<VectorClock<>::VC_ID>* shared_vc) const
{
  for (unsigned int i = 0; i < shared_vc->size(); ++i)
  {
    VectorClock<>::VC_ID act_id = get_sh_id(i, shared_vc);
    VectorClock<>::Thread_Num act_th_num = VectorClock<>::make_th_num(act_id);

    if (act_id != 0 && t->get_tid() != act_th_num &&
        VectorClock<>::make_clock(act_id) >= t->get_clock_by_th_num(act_th_num)) {
      return act_th_num;
    }
  }
  return 0;
}

// TODO: optimize using vector instructions
std::vector<VectorClock<>::VC_ID>::iterator VarState::find_in_vec(
    VectorClock<>::Thread_Num th_num, xvector<VectorClock<>::VC_ID>* shared_vc) const
{// TODO: make it public & add the shared_vc argument
  auto it = shared_vc->begin();
  auto it_end = shared_vc->end();
  for (; it != it_end; ++it)
  {//made the shorter run
    if (VectorClock<>::make_th_num(*it) == th_num)
    {//we run a find here at each iteration !!!!!!!
      return it;
    }
  }
  return shared_vc->end();
}

/// if in read_shared state, then returns thread id of position pos in vector
/// clock
VectorClock<>::VC_ID VarState::get_sh_id(
    uint32_t pos, xvector<VectorClock<>::VC_ID>* shared_vc) const
{
  if (pos < shared_vc->size())
  {
    return (*shared_vc)[pos];
  }
  return 0;
}

/// return stored clock value, which belongs to ThreadState t, 0 if not
/// available
VectorClock<>::VC_ID VarState::get_vc_by_th_num(
    VectorClock<>::Thread_Num th_num, xvector<VectorClock<>::VC_ID>* shared_vc) const
{
  auto it = find_in_vec(th_num, shared_vc);
  if (it != shared_vc->end())
  {
    return *it;
  }
  return 0;
}

VectorClock<>::Clock VarState::get_clock_by_th_num(
    VectorClock<>::Thread_Num th_num, xvector<VectorClock<>::VC_ID>* shared_vc) const
{
  auto it = find_in_vec(th_num, shared_vc);
  if (it != shared_vc->end()) {
    return VectorClock<>::make_clock(*it);
  }
  return 0;
}


/**
 * \brief updates the var state because of an new read or write access through
 * an thread \todo check thread-safety
 */
 // void VarState::update(bool is_write, VectorClock<>::VC_ID id) {
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
 // void VarState::set_read_shared(VectorClock<>::VC_ID id)
 //{ // no need for find here, but we have to access hashmap shared_vcs
 //  shared_vc = std::make_unique<xvector<size_t>>();
 //  shared_vc->reserve(2);
 //  shared_vc->push_back(r_id);
 //  shared_vc->push_back(id);
 //
 //  r_id = VAR_NOT_INIT;
 //}
