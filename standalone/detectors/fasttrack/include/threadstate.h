#ifndef THREADSTATE_H
#define THREADSTATE_H
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

#include <atomic>
#include <memory>
#include "stacktrace.h"
#include "vectorclock.h"
#include "xvector.h"

/// implements a threadstate, holds the thread's vectorclock and the thread's id
/// (tid and act clock)
class ThreadState : public VectorClock<> { //TODO: why do we inherit from VectorClock ?
 private:
  /// holds the tid and the actual clock value -> lower 32 bits are clock, upper
  /// 32 are the tid
  std::atomic<VectorClock::VC_ID> id;
  StackTrace traceDepot;

 public:
  /// constructor of ThreadState object, initializes tid and clock
  /// copies the vector of parent thread, if a parent thread exists
  ThreadState(VectorClock::TID own_tid,
              const std::shared_ptr<ThreadState>& parent = nullptr);

  /// increases own clock value
  void inc_vc();

  /// returns own clock value (upper bits) & thread (lower bits)
  inline VectorClock::VC_ID return_own_id() const { return id; }

  /// returns thread id
  inline VectorClock::TID get_tid() const { return VectorClock::make_tid(id); }

  /// returns current clock
  inline VectorClock::Clock get_clock() const {
    return VectorClock::make_clock(id);
  }

  /// may be called after exitting of thread
  inline void delete_vector() { vc.clear(); }

  /// return stackDepot of this thread
  StackTrace& get_stackDepot() { return traceDepot; }
};

#endif  // !THREADSTATE_H
