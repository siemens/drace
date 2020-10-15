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
#include "vectorclock.h"
#include "xvector.h"

#include "PrefixTree_StackDepot.h"
#include "TreeDepot.h"
#include "stacktrace.h"

/// implements a threadstate, holds the thread's vectorclock and the thread's id
/// (tid and act clock)
class ThreadState : public VectorClock<> {
 private:
  /// holds the tid and the actual clock value -> lower 21 bits are clock and 11
  /// bits are TID
  VectorClock<>::VC_ID id;
  VectorClock<>::TID m_own_tid;

  TreeDepot traceDepot;
  phmap::flat_hash_map<size_t, std::pair<size_t, INode*>> _read_write;

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
    return VectorClock::MakeThreadNum(id);
  }

  /// returns current clock
  constexpr VectorClock::Clock get_clock() const {
    return VectorClock::MakeClock(id);
  }

  /// may be called after exitting of thread
  inline void delete_vector() { vc.clear(); }

  /// return stackDepot of this thread
  TreeDepot& getStackDepot() { return traceDepot; }

  mutable ipc::spinlock _read_write_lock;
  inline void set_read_write(size_t addr, size_t pc) {
    // PRINT_TID();
    // DEB_FUNCTION();  // REMOVE_ME

    std::lock_guard<ipc::spinlock> lg(_read_write_lock);
    auto it = _read_write.find(addr);
    if (it == _read_write.end()) {
      // TODO: maybe use std::move avoid copy on pair
      // should I do sth if traceDepot.GetCurrentElement() is nullptr??
      _read_write.insert({addr, {pc, traceDepot.GetCurrentElement()}});
    } else {
      it->second = {pc, traceDepot.GetCurrentElement()};
    }
  }

  std::deque<size_t> return_stack_trace(size_t address) const {
    // DEB_FUNCTION();  // REMOVE_ME

    std::lock_guard<ipc::spinlock> lg(
        _read_write_lock);  // needed in some cases as some points might
                            // reallocate
    auto it = _read_write.find(address);
    if (it != _read_write.end()) {
      auto data = it->second;
      return traceDepot.MakeTrace(data);
    }
    // A read/write operation was not tracked correctly => return empty
    // stacktrace
    return {};
  }
};
#endif  // !THREADSTATE_H
