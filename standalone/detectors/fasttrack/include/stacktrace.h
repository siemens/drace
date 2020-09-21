#ifndef _STACKTRACE_H_
#define _STACKTRACE_H_
/*
 * DRace, a dynamic data race detector
 *
 * Copyright 2020 Siemens AG
 *
 * Authors:
 *   Felix Moessbauer <felix.moessbauer@siemens.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <ipc/spinlock.h>
#include <parallel_hashmap/phmap.h>
#include <boost/graph/adjacency_list.hpp>
#include <deque>

/**
 * \brief Implements a stack depot capable to store callstacks
 *        with references to particular nodes.
 */
class StackTrace {
  /// Store the address along with each vertex
  struct VertexProperty {
    size_t addr;
  };
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
                                VertexProperty>
      StackTree;

  /// holds var_address, pc, stack_length
  phmap::flat_hash_map<size_t, std::pair<size_t, StackTree::vertex_descriptor>>
      _read_write;

  /// holds to complete stack tree
  /// is needed to create the stack trace in case of a race
  /// leafs of the tree which do not have pointer pointing to them may be
  /// deleted
  StackTree _local_stack;

  /// reference to the current stack element
  StackTree::vertex_descriptor _ce;

  /// reference to the root element
  StackTree::vertex_descriptor _root;

  uint16_t _pop_count = 0;

  mutable ipc::spinlock lock;

  /// re-construct a stack-trace from a bottom node to the root
  std::deque<size_t> make_trace(
      const std::pair<size_t, StackTree::vertex_descriptor>& data) const;

  /**
   * \brief cleanup unreferenced nodes in callstack tree
   * \warning very expensive
   */
  void clean();

 public:
  StackTrace() : _ce(boost::add_vertex({0}, _local_stack)), _root(_ce) {}

  /**
   * \brief pop the last element from the stack
   * \note precondition: stack is not empty
   * \note threadsafe
   */
  void pop_stack_element();

  /**
   * \brief push a new element to the stack depot
   * \note threadsafe
   */
  void push_stack_element(size_t element);

  /**
   * when a var is written or read, it copies the stack and adds the pc of the
   * r/w operation to be able to return the stack trace if a race was detected
   * \note threadsafe
   */
  void set_read_write(size_t addr, size_t pc);

  /**
   * \brief returns a stack trace of a clock for handing it over to drace
   * \note threadsafe
   */
  std::deque<size_t> return_stack_trace(size_t address) const;
};
#endif
