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
  static constexpr size_t multplier = 0x100000000ull;
  typedef size_t VC_ID;
  typedef uint32_t TID;
  typedef uint32_t Clock;
#endif
  typedef uint32_t th_num;//to be used later 

  /// vector clock which contains multiple thread ids, clocks
  phmap::flat_hash_map<TID, VC_ID> vc;

  /// return the thread id of the position pos of the vector clock
  TID get_thr(uint32_t pos) const {
    if (pos < vc.size()) {
      auto it = vc.begin();
      std::advance(it, pos);
      return it->first;
    }
    else {
      return 0;
    }
  };

  /// returns the no. of elements of the vector clock
  constexpr uint32_t get_length() { return vc.size(); };

  /// updates this.vc with values of other.vc, if they're larger -> one way
  /// update
  void update(VectorClock* other) {
    for (auto it = other->vc.begin(); it != other->vc.end(); it++) {
      if (it->second > get_id_by_tid(it->first)) {
        update(it->first, it->second);
      }
    }
  };

  //TODO: maybe use an rvalue reference ?
  /// updates this.vc with values of other.vc, if they're larger -> one way
  /// update
  void update(const VectorClock& other) {
    for (auto it = other.vc.begin(); it != other.vc.end(); it++) {
      if (it->second > get_id_by_tid(it->first)) {
        update(it->first, it->second);
      }
    }
  };

  /// updates vector clock entry or creates entry if non-existant
  void update(TID tid, VC_ID id) {
    auto it = vc.find(tid);
    if (it == vc.end()) {
      vc.insert({ tid, id });
    }
    else {
      if (it->second < id) {
        it->second = id;
      }
    }
  };

  /// deletes a vector clock entry, checks existance before
  void delete_vc(TID tid) { vc.erase(tid); }

  /**
   * \brief returns known clock of tid
   *        returns 0 if vc does not hold the tid
   */
  Clock get_clock_by_tid(TID tid) const {
    auto it = vc.find(tid);
    if (it != vc.end()) {
      return make_clock(it->second);
    }
    else {
      return 0;
    }
  }

  /// returns known whole id in vectorclock of tid
  VC_ID get_id_by_tid(TID tid) const {
    auto it = vc.find(tid);
    if (it != vc.end()) {
      return it->second;
    }
    else {
      return 0;
    }
  }

  /// returns the tid of the id
  static constexpr TID make_tid(VC_ID id) {
    return static_cast<TID>(id / multplier);
  }

  /// returns the clock of the id
  static constexpr Clock make_clock(VC_ID id) {
    return static_cast<Clock>(id % multplier);
  }

  /// creates an id with clock=0 from an tid
  static constexpr VC_ID make_id(TID tid) {
    return tid * VectorClock::multplier;
  }
};
#endif
