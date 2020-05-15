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

#include "parallel_hashmap/phmap.h"
/**
    Implements a VectorClock.
    Can hold arbitrarily much pairs of a Thread Id and the belonging clock
*/
template <class _al = std::allocator<std::pair<const size_t, size_t>>>
class VectorClock {
 public:
  /// by dividing the id with the multiplier one gets the tid, with modulo one
  /// gets the clock

// on 64 bit platform 64 bits can be used for a VC_ID on 32 bit only the half
#if COMPILE_X86
  static constexpr size_t multplier = 0x10000ull;
  typedef size_t VC_ID;
  typedef unsigned short int TID;
  typedef unsigned short int Clock;
#else
  // static constexpr size_t multplier = 0x100000000ull;
  typedef uint32_t VC_ID;
  typedef uint32_t TID;
  typedef uint32_t Clock;
  typedef uint32_t Thread_Num;  // to be used later
#endif

  /// vector clock which contains multiple thread ids, clocks
  phmap::flat_hash_map<Thread_Num, VC_ID> vc;

  /// returns the no. of elements of the vector clock
  constexpr uint32_t get_length() { return vc.size(); }

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
      return make_clock(it->second);
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
  static constexpr TID make_tid(VC_ID id) {
    Thread_Num key = id >> 22;
    auto it = thread_ids.find(static_cast<Thread_Num>(key));
    if (it != thread_ids.end()) {
      return static_cast<TID>(it->second);
    } else {  // we should never reach this one
      return -1;
    }
  }

  /// returns the clock of the id
  static constexpr Clock make_clock(VC_ID id) {
    return static_cast<Clock>(id & 0x3FFFFF);
  }

  /// returns the clock of the id
  static constexpr Thread_Num make_th_num(VC_ID id) {
    return static_cast<Thread_Num>(id >> 22);
  }

  static constexpr TID make_tid_from_th_num(
      Thread_Num th_num) {  // TODO: maybe? put some checks to see it's really there
    return static_cast<TID>(thread_ids[th_num]);
  }

  // TODO: create a queue of thread numbers;
  static Thread_Num thread_no;
  static phmap::flat_hash_map<VectorClock<>::Thread_Num, VectorClock<>::TID>
      thread_ids;

  /// creates an id with clock=0 from an tid
  static constexpr VC_ID make_id(TID tid) {
    thread_ids.emplace(thread_no, tid);
    VC_ID id = thread_no << 22;  // epoch is 0
    thread_no++;
    return id;
  }
};
uint32_t VectorClock<>::thread_no = 1;
phmap::flat_hash_map<VectorClock<>::Thread_Num, VectorClock<>::TID>
    VectorClock<>::thread_ids;
#endif
