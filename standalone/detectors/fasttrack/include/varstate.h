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
#ifndef VARSTATE_H
#define VARSTATE_H

#include <atomic>
#include <memory>
#include "threadstate.h"
#include "vectorclock.h"
#include "xvector.h"

/**
 * \brief stores information about a memory location
 * \note does not store the address to avoid redundant information
 */
class VarState
{
 public:
  static constexpr uint32_t VAR_NOT_INIT = 0;
  VectorClock<>::VC_ID w_id { VAR_NOT_INIT };  // these are epochs
  /// local clock of last read
  VectorClock<>::VC_ID r_id { VAR_NOT_INIT }; // now they should be 32 bits 

  // TODO: remove the const for copy constructor if we move to flat_hash_map
  const uint16_t size;  // TODO: make size smaller

  explicit inline VarState(uint16_t var_size) : size(var_size) {}

  /// evaluates for write/write races through this and and access through t
  bool is_ww_race(ThreadState* t) const;

  /// evaluates for write/read races through this and and access through t
  bool is_wr_race(ThreadState* t) const;

  /// evaluates for read-exclusive/write races through this and and access
  /// through t
  bool is_rw_ex_race(ThreadState* t) const;

  /// returns id of last write access
  inline VectorClock<>::VC_ID get_write_id() const { return w_id; }

  /// returns id of last read access (when read is not shared)
  inline VectorClock<>::VC_ID get_read_id() const { return r_id; }

  /// return tid of thread which last wrote this var
  inline VectorClock<>::TID get_w_tid() const { return VectorClock<>::make_tid(w_id); }

  /// return tid of thread which last read this var, if not read shared
  inline VectorClock<>::TID get_r_tid() const { return VectorClock<>::make_tid(r_id); }

  /// returns clock value of thread of last write access
  inline VectorClock<>::Clock get_w_clock() const { return VectorClock<>::make_clock(w_id); }

  /// returns clock value of thread of last read access (returns 0 when read is shared)
  inline VectorClock<>::Clock get_r_clock() const { return VectorClock<>::make_clock(r_id); }

  std::vector<VectorClock<>::VC_ID>::iterator find_in_vec(VectorClock<>::TID tid, xvector<VectorClock<>::VC_ID>* shared_vc) const;
  VectorClock<>::Clock get_clock_by_thr(VectorClock<>::TID tid, xvector<VectorClock<>::VC_ID>* shared_vc) const;
  VectorClock<>::VC_ID get_vc_by_thr(VectorClock<>::TID tid, xvector<VectorClock<>::VC_ID>* shared_vc) const;
  VectorClock<>::VC_ID get_sh_id(uint32_t pos, xvector<VectorClock<>::VC_ID>* shared_vc) const;
  VectorClock<>::TID is_rw_sh_race(ThreadState* t, xvector<VectorClock<>::VC_ID>* shared_vc) const;

  /// returns true when read is shared
  // bool is_read_shared() const { return (shared_vc == false) ? false : true; }

  /// updates the var state because of an new read or write access through an
  /// thread
  //void update(bool is_write, size_t id);

  /// sets read state to shared
  //void set_read_shared(size_t id);

  /// if in read_shared state, then returns id of position pos in vector clock
  // VectorClock<>::VC_ID get_sh_id(uint32_t pos) const;

  /// return stored clock value, which belongs to ThreadState t, 0 if not
  /// available
  // VectorClock<>::VC_ID get_vc_by_thr(VectorClock<>::TID t) const;

  // VectorClock<>::Clock get_clock_by_thr(VectorClock<>::TID t) const;
  /// finds the entry with the tid in the shared vectorclock
  // auto find_in_vec(VectorClock<>::TID tid) const;

  /// evaluates for read-shared/write races through this and and access through
  /// "t"
  // VectorClock<>::TID is_rw_sh_race(ThreadState* t) const;

};
#endif  // !VARSTATE_H

//----------------------------------------------------------------------------------------------
// added by me
// explicit VarState() = default;
// VarState(const VarState& v)
//{// copy constructor
//  this->shared_vc = v.shared_vc;
//  this->w_id = v.w_id;
//  this->r_id = v.r_id;
//}
// VarState(VarState&& v)
//{// move constructor
//  this->shared_vc = v.shared_vc;
//  this->w_id = v.w_id;
//  this->r_id = v.r_id;
//}
// VarState& operator=(const VarState& other)
//{// copy assignment operator
//  this->shared_vc = other.shared_vc;
//  this->w_id = other.w_id;
//  this->r_id = other.r_id;
//  return *this;
//}
// VarState& operator=(VarState&& other)
//{// move assignment operator
//  this->shared_vc = other.shared_vc;
//  this->w_id = other.w_id;
//  this->r_id = other.r_id;
//  return *this;
//}
//----------------------------------------------------------------------------------------------
