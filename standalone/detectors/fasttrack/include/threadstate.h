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
#include "stacktrie.h"

/// implements a threadstate, holds the thread's vectorclock and the thread's id
/// (tid and act clock)
class ThreadState : public VectorClock<> {
 private:
  /// holds the tid and the actual clock value -> lower 18 bits are clock and 14
  /// bits are TID
  VectorClock<>::VC_ID id;
  StackTraceTrie traceDepot;
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
  StackTraceTrie& get_stackDepot() { return traceDepot; }

  phmap::flat_hash_map<size_t, size_t> _read_write;

  ///// reference to the current stack element
  //size_t _ce;

  void set_read_write(size_t addr, size_t pc) {
  //TODO: need a lock
    auto it = _read_write.find(addr);
    if (it == _read_write.end()) {
      _read_write.emplace(addr, pc);
    } else {
      it->second = pc;
    }
  }
};
#endif  // !THREADSTATE_H
