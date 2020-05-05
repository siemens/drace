#ifndef STACKTRACE_H
#define STACKTRACE_H 1
#pragma once
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

// TODO: maybe create a common interface for all types of callstacks
/**
 * \brief Implements a stack depot capable to store callstacks
 *        with references to particular nodes.
 */
class StackTrace {
 public:
  /// Store the address along with each vertex
  struct VertexProperty {
    size_t addr;
  };
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
                                VertexProperty>
      StackTree;

 private:
  /// holds to complete stack tree
  /// is needed to create the stack trace in case of a race
  /// leafs of the tree which do not have pointer pointing to them may be
  /// deleted
  StackTree local_stack;

  /// reference to the current stack element
  StackTree::vertex_descriptor curr_elem;

  /// reference to the root element
  StackTree::vertex_descriptor root_node;

  uint16_t pop_count = 0;

  mutable ipc::spinlock lock;

  /**
   * \note locking was moved to ThreadState
   * \note Locking is necessary if and only if elements are removed from the
   * tree. As long as no elements are removed locking is not necessary: mutable
   * ipc::spinlock lock;
   */

  /**
   * \brief cleanup unreferenced nodes in callstack tree
   * \warning very expensive
   */
  void clean();

 public:
  /// re-construct a stack-trace from a bottom node to the root
  std::deque<size_t> make_trace(
      const std::pair<size_t, StackTree::vertex_descriptor>& data) const;

  StackTrace()
      : curr_elem(boost::add_vertex({0}, local_stack)), root_node(curr_elem) {}

  /**
   * \brief pop the last element from the stack
   * \note precondition: stack is not empty
   * \note threadsafe
   */
  void remove_function_element();

  /**
   * \brief push a new element to the stack depot
   * \note threadsafe
   */
  void insert_function_element(size_t element);

  StackTree::vertex_descriptor get_current_element() const;
};
#endif