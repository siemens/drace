#ifndef THREADSTATE_H
#define THREADSTATE_H 1
#pragma once
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

#include "stacktrace.h"
#include "vectorclock.h"
#include "xvector.h"

// TODO: eliminate magic number, value
/**
 * \note Number 5 from here should be equal to vars_number_of_sub_maps
 */
template <class K, class V>
using parallel_flat_hash_map = phmap::parallel_node_hash_map<
    K, V, phmap::container_internal::hash_default_hash<K>,
    phmap::container_internal::hash_default_eq<K>,
    std::allocator<std::pair<const K, V>>, 5, ipc::spinlock>;

/**
 * \class ThreadState
 * \brief implements a threadstate (see FastTrack2 algorithm), holds the
 * thread's vectorclock and the thread's id
 */
class ThreadState : public VectorClock<> {
 private:
  /// holds the tid and the actual clock value -> lower 21 bits are clock and 11
  /// bits are TID
  VectorClock<>::VC_EPOCH id;

  /// cache the thread ID
  VectorClock<>::TID own_tid;

  /**
   * \note a more performant, cache efficient way to store call stack
   * informative is the TreeDepot which implements the functionality of a cache
   * efficient prefix tree (currently only experimental use): TreeDepot
   * traceDepot; with parallel_flat_hash_map<<size_t, std::pair<size_t, INode*>>
   * read_write;
   */

  /// holds call stack representation
  StackTrace traceDepot;
  struct VertexProperty {
    size_t addr;
  };
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
                                VertexProperty>
      StackTree;

 public:
  /**
   * \note We currently use the parallel version for the function
   * remove_memory_addresses(), policy flag_remove_by_prunning_hash_map, which
   * requires dropping a sub hash map. Also not using make_shared in
   * create_thread due to "object allocated on the heap may not be aligned 64"
   */
  /// holds var_address, pc, stack_length
  parallel_flat_hash_map<size_t,
                         std::pair<size_t, StackTree::vertex_descriptor>>
      read_write;

  /// constructor of ThreadState object, initializes tid and clock
  /// copies the vector of parent thread, if a parent thread exists
  ThreadState(VectorClock::TID own_tid,
              const std::shared_ptr<ThreadState>& parent = nullptr);

  /// increases own clock value
  void inc_vc();

  /// returns own clock value (upper bits) & thread (lower bits)
  constexpr VectorClock::VC_EPOCH return_own_id() const { return id; }

  /// returns thread id
  constexpr VectorClock::TID get_tid() const { return own_tid; }

  /// returns the allocated thread number
  constexpr VectorClock::ThreadNum get_th_num() const {
    return VectorClock::make_thread_num(id);
  }

  /// returns current clock
  constexpr VectorClock::Clock get_clock() const {
    return VectorClock::make_clock(id);
  }

  /// may be called after exitting of thread
  inline void delete_vector() { vc.clear(); }

  /// return stackDepot of this thread
  StackTrace& get_stack_depot() { return traceDepot; }

  /**
   * when a var is written or read, it maps the element of the stack trace
   * corresponding to the current pc of the r/w
   * operation to the respective memory address  to be able to return the stack
   * trace if a race was detected
   * \note threadsafe
   */
  void set_read_write(std::size_t addr, std::size_t pc);

  /**
   * \brief returns a stack trace of a memory location for handing it over to
   * drace \note theadsafe
   */
  std::deque<size_t> return_stack_trace(std::size_t address) const;
};
#endif  // !THREADSTATE_H