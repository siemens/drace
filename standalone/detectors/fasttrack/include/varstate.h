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
  static constexpr uint32_t READ_SHARED = -1;

  // TODO: make these private and make setters & getters for it
  /// local clock of last read
  VectorClock<>::VC_ID _writeID{VAR_NOT_INIT};
  // these are th_num (first 11 bits) + 21 bits epoch

  /// local clock of last read
  VectorClock<>::VC_ID _readID{VAR_NOT_INIT};

  VarState() = default;

  /// returns id of last write access
  constexpr VectorClock<>::VC_ID GetWriteID() const { return _writeID; }

  /// returns id of last read access (when read is not shared)
  constexpr VectorClock<>::VC_ID GetReadID() const { return _readID; }

  /// return tid of thread which last wrote this var
  constexpr VectorClock<>::TID GetWriteThreadID() const {
    return VectorClock<>::MakeThreadID(_writeID);
  }

  /// return tid of thread which last read this var, if not read shared
  constexpr VectorClock<>::TID GetReadThreadID() const {
    return VectorClock<>::MakeThreadID(_readID);
  }

  constexpr VectorClock<>::Thread_Num GetWriteThreadNum() const {
    return VectorClock<>::MakeThreadNum(_writeID);
  }
  constexpr VectorClock<>::Thread_Num GetReadThreadNum() const {
    return VectorClock<>::MakeThreadNum(_readID);
  }

  /// returns clock value of thread of last write access
  constexpr VectorClock<>::Clock GetWriteClock() const {
    return VectorClock<>::MakeClock(_writeID);
  }

  /// returns clock value of thread of last read access (returns 0 when read is
  /// shared)
  constexpr VectorClock<>::Clock GetReadClock() const{
    return VectorClock<>::MakeClock(_readID);
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
