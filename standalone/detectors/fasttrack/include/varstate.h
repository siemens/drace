#ifndef VARSTATE_HEADER_H
#define VARSTATE_HEADER_H

/*
 * DRace, a dynamic data race detector
 *
 * Copyright 2020 Siemens AG
 *
 * Authors:
 *   Felix Moessbauer <felix.moessbauer@siemens.com>
 *   Mihai Robescu <mihai-gabriel.robescu@siemens.com>
 *
 * SPDX-License-Identifier: MIT
 */

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

 private:
  /// local clock of last write; this is th_num (first 11 bits) + 21 bits clock
  /// value
  VectorClock<>::VC_EPOCH write_epoch{VAR_NOT_INIT};

  /// local clock of last read; this is th_num (first 11 bits) + 21 bits clock
  /// value
  VectorClock<>::VC_EPOCH read_epoch{VAR_NOT_INIT};

 public:
  VarState() = default;

  /// returns id of last write access
  constexpr VectorClock<>::VC_EPOCH get_write_epoch() const {
    return write_epoch;
  }

  /// returns id of last read access (when read is not shared)
  constexpr VectorClock<>::VC_EPOCH get_read_epoch() const {
    return read_epoch;
  }

  /// sets id of last write access
  constexpr void set_write_epoch(VectorClock<>::VC_EPOCH epoch) {
    write_epoch = epoch;
  }

  /// sets id of last read access (when read is not shared)
  constexpr void set_read_epoch(VectorClock<>::VC_EPOCH epoch) {
    read_epoch = epoch;
  }

  /// return tid of thread which last wrote to this memory location
  constexpr VectorClock<>::TID get_write_thread_id() const {
    return VectorClock<>::make_thread_id(write_epoch);
  }

  /// return tid of thread which last read this memory location, if not read
  /// shared
  constexpr VectorClock<>::TID get_read_thread_id() const {
    return VectorClock<>::make_thread_id(read_epoch);
  }

  /// return thread number of thread which last read this memory location, if
  /// not read shared
  constexpr VectorClock<>::ThreadNum get_write_thread_num() const {
    return VectorClock<>::make_thread_num(write_epoch);
  }

  /// return thread number of thread which last wrote to this memory location,
  /// if not read shared
  constexpr VectorClock<>::ThreadNum get_read_thread_num() const {
    return VectorClock<>::make_thread_num(read_epoch);
  }

  /// returns clock value of thread of last write access
  constexpr VectorClock<>::Clock get_write_clock() const {
    return VectorClock<>::make_clock(write_epoch);
  }

  /// returns clock value of thread of last read access (returns 0 when read is
  /// shared)
  constexpr VectorClock<>::Clock get_read_clock() const {
    return VectorClock<>::make_clock(read_epoch);
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
  VectorClock<>::ThreadNum VarState::is_rw_sh_race(
      ThreadState* t, xvector<VectorClock<>::VC_EPOCH>* shared_vc) const;

  /// finds the entry with the th_num in the shared vectorclock
  static std::vector<VectorClock<>::VC_EPOCH>::iterator VarState::find_in_vec(
      VectorClock<>::ThreadNum th_num,
      xvector<VectorClock<>::VC_EPOCH>*
          shared_vc);  // made static as it does not depend on the VarState
                       // instance

  /// if in read_shared state, then returns id of position pos in vector clock
  VectorClock<>::VC_EPOCH VarState::get_sh_id(
      uint32_t pos, xvector<VectorClock<>::VC_EPOCH>* shared_vc) const;

  /// return stored epoch (th_num + clock) value, which belongs to ThreadNum
  /// th_num or 0 if not available
  VectorClock<>::VC_EPOCH VarState::get_vc_by_th_num(
      VectorClock<>::ThreadNum th_num,
      xvector<VectorClock<>::VC_EPOCH>* shared_vc) const;

  /// return stored clock value, which belongs to ThreadNum th_num or 0 if not
  /// available
  VectorClock<>::Clock VarState::get_clock_by_th_num(
      VectorClock<>::ThreadNum th_num,
      xvector<VectorClock<>::VC_EPOCH>* shared_vc) const;
};
#endif  // !VARSTATE_HEADER_H
