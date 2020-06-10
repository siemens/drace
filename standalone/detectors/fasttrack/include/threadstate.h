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
class ThreadState : public VectorClock<> {
 private:
  /// holds the tid and the actual clock value -> lower 18 bits are clock and 14
  /// bits are TID
  VectorClock<>::VC_ID id;
  StackTrace traceDepot;
  VectorClock<>::TID m_own_tid;

 public:
  /// constructor of ThreadState object, initializes tid and clock
  /// copies the vector of parent thread, if a parent thread exists
  ThreadState(VectorClock::TID own_tid,
              const std::shared_ptr<ThreadState>& parent = nullptr);

  /// increases own clock value
  void inc_vc();

  /// returns own clock value (upper bits) & thread (lower bits)
  constexpr VectorClock::VC_ID return_own_id() const { return id; }

  /// returns thread id
  constexpr VectorClock::TID get_tid() const { return m_own_tid; }
  // we eliminate one more find if we cache the own_tid;

  constexpr VectorClock::Thread_Num get_th_num() const {
    return VectorClock::make_th_num(id);
  }

  /// returns current clock
  constexpr VectorClock::Clock get_clock() const {
    return VectorClock::make_clock(id);
  }

  /// may be called after exitting of thread
  inline void delete_vector() { vc.clear(); }

  /// return stackDepot of this thread
  StackTrace& get_stackDepot() { return traceDepot; }
};
#endif  // !THREADSTATE_H
