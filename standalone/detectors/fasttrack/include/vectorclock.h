#ifndef VECTORCLOCK_H
#define VECTORCLOCK_H
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

#include <stack>
#include "parallel_hashmap/phmap.h"
/*
---------------------------------------------------------------------
Implements a VectorClock.
Can hold an arbitrarily number pairs of Thread Numbers (replacement for
Thread IDs) and the belonging clock
---------------------------------------------------------------------
*/

template <class _al = std::allocator<std::pair<const size_t, size_t>>>
class VectorClock {
 public:
  typedef uint32_t VC_ID;  // first 11 bits represent the TID / thread number
                           // and last 21 bits the Clock
  static constexpr uint32_t CLOCK_BITS = 21;
  static constexpr uint32_t CLOCK_MASK = (1 << CLOCK_BITS) - 1;  // 0x1FFFFF;
  // 32 comes from the fact that uint32_t has 32 bits
  static constexpr uint32_t MAX_TH_NUM = (1 << (32 - CLOCK_BITS)) - 1;
  typedef uint32_t TID;
  typedef uint32_t Clock;
  typedef uint32_t Thread_Num;

  /// vector clock which contains multiple thread ids, clocks
  phmap::flat_hash_map<Thread_Num, VC_ID> vc;

  /// returns the no. of elements of the vector clock
  constexpr uint32_t get_length() { return vc.size(); }

  Clock get_min_clock() {
    Clock min_clock = -1;
    auto it = this->vc.begin();
    auto it_end = this->vc.end();

    for (; it != it_end; it++) {
      Clock tmp = MakeClock(it->second);
      if (tmp < min_clock) {
        min_clock = tmp;
      }
    }
    return min_clock;
  }

  /// updates this.vc with values of other.vc, if they're larger -> one way
  /// update
  void update(VectorClock* other) {
    for (auto it = other->vc.begin(); it != other->vc.end(); it++) {
      if (it->second > get_id_by_th_num(it->first)) {
        update(it->first, it->second);
      }
    }
  }

  // TODO: maybe use an rvalue reference ?
  /// updates this.vc with values of other.vc, if they're larger -> one way
  /// update
  void update(const VectorClock& other) {
    for (auto it = other.vc.begin(); it != other.vc.end(); it++) {
      if (it->second > get_id_by_th_num(it->first)) {
        update(it->first, it->second);
      }
    }
  }

  /// updates vector clock entry or creates entry if non-existant
  void update(Thread_Num th_num, VC_ID id) {
    auto it = vc.find(th_num);
    if (it == vc.end()) {
      vc.insert({th_num, id});
    } else {
      if (it->second < id) {
        it->second = id;
      }
    }
  }

  /// deletes a vector clock entry, checks existance before
  void delete_vc(Thread_Num th_num) { vc.erase(th_num); }

  /**
   * \brief returns known clock of tid
   *        returns 0 if vc does not hold the tid
   */
  Clock get_clock_by_th_num(Thread_Num th_num) const {
    auto it = vc.find(th_num);
    if (it != vc.end()) {
      return MakeClock(it->second);
    } else {
      return 0;
    }
  }

  /// returns known whole id in vectorclock of tid
  VC_ID get_id_by_th_num(Thread_Num th_num) const {
    auto it = vc.find(th_num);
    if (it != vc.end()) {
      return it->second;
    } else {
      return 0;
    }
  }

  /// returns the tid of the id
  static constexpr TID MakeThreadID(VC_ID id) {
    Thread_Num th_num = id >> CLOCK_BITS;
    return make_tid_from_th_num(th_num);
  }

  /// returns the clock of the id
  static constexpr Clock MakeClock(VC_ID id) {
    return static_cast<Clock>(id & CLOCK_MASK);
  }

  /// returns the clock of the id
  static constexpr Thread_Num MakeThreadNum(VC_ID id) {
    return static_cast<Thread_Num>(id >> CLOCK_BITS);
  }

  static constexpr TID make_tid_from_th_num(Thread_Num th_num) {
    auto it = thread_ids.find(th_num);
    if (it != thread_ids.end()) {
      return static_cast<TID>(it->second);
    } else {  // we should never reach this one
      return -1;
    }
  }

  // TODO: put them at the top
  static std::stack<Thread_Num> thread_nums;
  static phmap::flat_hash_map<VectorClock<>::Thread_Num, VectorClock<>::TID>
      thread_ids;

  /// creates an id with clock=0 from an tid
  static constexpr VC_ID make_id(TID tid) {
    //---------------------------------------------------------------------
    // TODO: for stack functionality we would still have to remove everything
    // belonging to a thread when it is joined -> really slow.
    // we can work with ever increasing numbers, as the stack functionality is
    // only useful for optimization/stacktrace
    //---------------------------------------------------------------------
    Thread_Num thread_no = thread_nums.top();
    thread_nums.pop();

    // checking for empty stack, no more thread numbers left
    // bool empty_stack = false;
    // if (thread_nums.empty()) {
    //  empty_stack = true;
    //}
    // while (empty_stack) {
    //  if (!thread_nums.empty()) empty_stack = false;
    //}

    thread_ids.emplace(thread_no, tid);
    VC_ID id = thread_no << CLOCK_BITS;  // epoch is 0
    return id;
  }

 private:
  static bool _thread_no_initiliazation;
  static bool ThreadNoInitialization() {
    for (int i = MAX_TH_NUM; i >= 1;
         --i) {  //!! thread numbers start from 1 because of is_rw_sh_race
      thread_nums.emplace(i);
    }
    return true;
  }
};
std::stack<VectorClock<>::Thread_Num> VectorClock<>::thread_nums;
bool VectorClock<>::_thread_no_initiliazation =
    VectorClock<>::ThreadNoInitialization();
phmap::flat_hash_map<VectorClock<>::Thread_Num, VectorClock<>::TID>
    VectorClock<>::thread_ids;
#endif
