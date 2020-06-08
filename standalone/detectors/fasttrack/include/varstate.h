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
class VarState {
 public:
  static constexpr uint32_t VAR_NOT_INIT = 0;
  static constexpr uint32_t R_ID_SHARED = -1;

  // TODO: make these private and make setters & getters for it
  /// local clock of last read
  VectorClock<>::VC_ID w_id{VAR_NOT_INIT};
  // these are th_num (first 14 bits) + 18 bits epoch

  /// local clock of last read
  VectorClock<>::VC_ID r_id{VAR_NOT_INIT};

  VarState() = default;

  /// returns id of last write access
  constexpr VectorClock<>::VC_ID get_write_id() const { return w_id; }

  /// returns id of last read access (when read is not shared)
  constexpr VectorClock<>::VC_ID get_read_id() const { return r_id; }

  /// return tid of thread which last wrote this var
  constexpr VectorClock<>::TID get_w_tid() const {
    return VectorClock<>::make_tid(w_id);
  }

  constexpr VectorClock<>::Thread_Num get_w_th_num() const {
    return VectorClock<>::make_th_num(w_id);
  }
  constexpr VectorClock<>::Thread_Num get_r_th_num() const {
    return VectorClock<>::make_th_num(r_id);
  }

  /// return tid of thread which last read this var, if not read shared
  constexpr VectorClock<>::TID get_r_tid() const {
    return VectorClock<>::make_tid(r_id);
  }

  /// returns clock value of thread of last write access
  constexpr VectorClock<>::Clock get_w_clock() const {
    return VectorClock<>::make_clock(w_id);
  }

  /// returns clock value of thread of last read access (returns 0 when read is
  /// shared)
  constexpr VectorClock<>::Clock get_r_clock() const{
    return VectorClock<>::make_clock(r_id);
  }

  /// evaluates for write/write races through this and and access through t
  bool is_ww_race(ThreadState* t) const;

  /// evaluates for write/read races through this and and access through t
  bool is_wr_race(ThreadState* t) const;

  /// evaluates for read-exclusive/write races through this and and access
  /// through t
  bool is_rw_ex_race(ThreadState* t) const;

  /// evaluates for read-shared/write races through this and and access through
  /// "t"
  VectorClock<>::Thread_Num VarState::is_rw_sh_race(
      ThreadState* t, xvector<VectorClock<>::VC_ID>* shared_vc) const;

  /// finds the entry with the th_num in the shared vectorclock
  static std::vector<VectorClock<>::VC_ID>::iterator VarState::find_in_vec(
      VectorClock<>::Thread_Num th_num,
      xvector<VectorClock<>::VC_ID>*
          shared_vc);  // made static as it does not depend on the VarState
                       // instance

  /// if in read_shared state, then returns id of position pos in vector clock
  VectorClock<>::VC_ID VarState::get_sh_id(
      uint32_t pos, xvector<VectorClock<>::VC_ID>* shared_vc) const;

  /// return stored clock value, which belongs to ThreadState t, 0 if not
  /// available
  VectorClock<>::VC_ID VarState::get_vc_by_th_num(
      VectorClock<>::Thread_Num th_num,
      xvector<VectorClock<>::VC_ID>* shared_vc) const;

  VectorClock<>::Clock VarState::get_clock_by_th_num(
      VectorClock<>::Thread_Num th_num,
      xvector<VectorClock<>::VC_ID>* shared_vc) const;
};
#endif  // !VARSTATE_H
