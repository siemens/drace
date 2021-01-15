#ifndef VECTORCLOCK_HEADER_H
#define VECTORCLOCK_HEADER_H 1
#pragma once

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
 * \class VectorClock
 * \brief A vector clock can hold an arbitrarily number pairs of Thread Numbers
 * (replacement for Thread IDs) and the corresponding clock value. It also
 * provides the thread number functionality, such that each thread uses a number
 * instead of the entire 32 bit ID
 */
template <class _al = std::allocator<std::pair<const size_t, size_t>>>
class VectorClock {
 public:
  /**
   * \typedef VC_EPOCH
   * \brief first 11 bits represent the TID / thread number
   * and last 21 bits the Clock
   */
  typedef uint32_t VC_EPOCH;
  static constexpr uint32_t CLOCK_BITS = 21;
  static constexpr uint32_t CLOCK_MASK = (1 << CLOCK_BITS) - 1;  // 0x1FFFFF;
  // 32 comes from the fact that uint32_t has 32 bits
  static constexpr uint32_t MAX_TH_NUM = (1 << (32 - CLOCK_BITS)) - 1;
  typedef uint32_t TID;
  typedef uint32_t Clock;
  typedef uint32_t ThreadNum;

  /// vector clock which contains multiple thread ids, clocks
  phmap::flat_hash_map<ThreadNum, VC_EPOCH> vc;

  static ThreadNum thread_no;
  static phmap::flat_hash_map<VectorClock<>::ThreadNum, VectorClock<>::TID>
      thread_ids;

  /// returns the no. of elements of the vector clock
  constexpr size_t get_length() { return vc.size(); }

  Clock get_min_clock() const {
    Clock min_clock = -1;
    auto it = this->vc.begin();
    auto it_end = this->vc.end();

    for (; it != it_end; it++) {
      Clock tmp = make_clock(it->second);
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

  /// updates this.vc with values of other.vc, if they're larger -> one way
  /// update
  void update(const VectorClock& other) {
    for (auto it = other.vc.begin(); it != other.vc.end(); it++) {
      if (it->second > get_id_by_th_num(it->first)) {
        update(it->first, it->second);
      }
    }
  }

  // maybe use an rvalue reference ?
  void update(VectorClock&& other) { this.vc = std::move(other.vc); }

  /// updates vector clock entry or creates entry if non-existant
  void update(ThreadNum th_num, VC_EPOCH id) {
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
  void delete_vc(ThreadNum th_num) { vc.erase(th_num); }

  /**
   * \brief returns known clock of tid
   *        returns 0 if vc does not hold the tid
   */
  Clock get_clock_by_th_num(ThreadNum th_num) const {
    auto it = vc.find(th_num);
    if (it != vc.end()) {
      return make_clock(it->second);
    } else {
      return 0;
    }
  }

  /// returns known whole id in vectorclock of tid
  VC_EPOCH get_id_by_th_num(ThreadNum th_num) const {
    auto it = vc.find(th_num);
    if (it != vc.end()) {
      return it->second;
    } else {
      return 0;
    }
  }

  /// returns the tid of the epoch
  static constexpr TID make_thread_id(VC_EPOCH epoch) {
    ThreadNum th_num = epoch >> CLOCK_BITS;
    return make_tid_from_th_num(th_num);
  }

  /// returns the clock of the epoch
  static constexpr Clock make_clock(VC_EPOCH epoch) {
    return static_cast<Clock>(epoch & CLOCK_MASK);
  }

  /// returns the thread number from the epoch
  static constexpr ThreadNum make_thread_num(VC_EPOCH epoch) {
    return static_cast<ThreadNum>(epoch >> CLOCK_BITS);
  }

  /// returns the corresponding thread id for a given thread num
  static constexpr TID make_tid_from_th_num(ThreadNum th_num) {
    auto it = thread_ids.find(th_num);
    if (it != thread_ids.end()) {
      return static_cast<TID>(it->second);
    } else {  // we should never reach this one
      return -1;
    }
  }

  /// creates an epoch with clock = 0 from an tid; linearly increasing thread
  /// nums
  static constexpr VC_EPOCH make_epoch(TID tid) {
    thread_ids.emplace(thread_no, tid);
    VC_EPOCH id = thread_no << CLOCK_BITS;
    thread_no++;
    if (thread_no >= MAX_TH_NUM) {
      /**
       * \note MAX_TH_NUM is the maximum number of threads. Afterwards we'll
       * have to overwrite them if thread_no goes over MAX_TH_NUM => queue
       * functionality;
       * linearly increasing thread_nums are good as we don't want to keep to
       * check VarStates of a finishing thread every time
       */

      thread_no = 1;
    }
    return id;
  }
};
VectorClock<>::ThreadNum VectorClock<>::thread_no = 1;
phmap::flat_hash_map<VectorClock<>::ThreadNum, VectorClock<>::TID>
    VectorClock<>::thread_ids;
#endif