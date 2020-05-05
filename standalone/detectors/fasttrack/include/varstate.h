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
  /// contains read_shared case all involved threads and clocks
  std::shared_ptr<std::vector<size_t>> shared_vc{nullptr};
  //made this shared & switched to vector => needed for the copy constructor

  /// the upper half of the bits are the thread id the lower half is the clock of the thread
  std::atomic<VectorClock<>::VC_ID> w_id{VAR_NOT_INIT}; //these are epochs
  /// local clock of last read
  std::atomic<VectorClock<>::VC_ID> r_id{VAR_NOT_INIT};

  /// finds the entry with the tid in the shared vectorclock
  auto find_in_vec(VectorClock<>::TID tid) const;

 public:
  static constexpr int VAR_NOT_INIT = 0;

  /**
   * \brief spinlock to ensure mutually-excluded access to a single VarState
   * instance
   */
  ipc::spinlock lock; //TODO: pool of spinlocks instead of one per Varstate 

  // var size 
  uint16_t size;// TODO: make size smaller
  //TODO: remove the const for copy constructor if we move to flat_hash_map 

//----------------------------------------------------------------------------------------------
  //added by me
  explicit VarState() = default;
  VarState(const VarState& v){
    this->shared_vc = v.shared_vc;
    this->w_id = v.w_id.load();
    this->r_id = v.r_id.load();
    this->size = v.size;
  } //= default; //copy constructor
  VarState& operator= (const VarState& other) {//copy assignment operator
    this->shared_vc = other.shared_vc;
    this->w_id = other.w_id.load();
    this->r_id = other.r_id.load();
    return *this;
  }
//----------------------------------------------------------------------------------------------
  
  explicit inline VarState(uint16_t var_size) : size(var_size) {}

  /// evaluates for write/write races through this and and access through t
  bool is_ww_race(ThreadState* t) const;

  /// evaluates for write/read races through this and and access through t
  bool is_wr_race(ThreadState* t) const;

  /// evaluates for read-exclusive/write races through this and and access
  /// through t
  bool is_rw_ex_race(ThreadState* t) const;

  /// evaluates for read-shared/write races through this and and access through
  /// t
  VectorClock<>::TID is_rw_sh_race(ThreadState* t) const;

  /// returns id of last write access
  inline VectorClock<>::VC_ID get_write_id() const {
    return w_id.load(std::memory_order_relaxed);
  }

  /// returns id of last read access (when read is not shared)
  inline VectorClock<>::VC_ID get_read_id() const {
    return r_id.load(std::memory_order_relaxed);
  }

  /// return tid of thread which last wrote this var
  inline VectorClock<>::TID get_w_tid() const {
    return VectorClock<>::make_tid(w_id.load(std::memory_order_relaxed));
  }

  /// return tid of thread which last read this var, if not read shared
  inline VectorClock<>::TID get_r_tid() const {
    return VectorClock<>::make_tid(r_id.load(std::memory_order_relaxed));
  }

  /// returns clock value of thread of last write access
  inline VectorClock<>::Clock get_w_clock() const {
    return VectorClock<>::make_clock(w_id.load(std::memory_order_relaxed));
  }

  /// returns clock value of thread of last read access (returns 0 when read is
  /// shared)
  inline VectorClock<>::Clock get_r_clock() const {
    return VectorClock<>::make_clock(r_id.load(std::memory_order_relaxed));
  }

  /// returns true when read is shared
  bool is_read_shared() const { return (shared_vc == nullptr) ? false : true; }

  /// updates the var state because of an new read or write access through an
  /// thread
  void update(bool is_write, size_t id);

  /// sets read state to shared
  void set_read_shared(size_t id);

  /// if in read_shared state, then returns id of position pos in vector clock
  VectorClock<>::VC_ID get_sh_id(uint32_t pos) const;

  /// return stored clock value, which belongs to ThreadState t, 0 if not
  /// available
  VectorClock<>::VC_ID get_vc_by_thr(VectorClock<>::TID t) const;

  VectorClock<>::Clock get_clock_by_thr(VectorClock<>::TID t) const;
};
#endif  // !VARSTATE_H
