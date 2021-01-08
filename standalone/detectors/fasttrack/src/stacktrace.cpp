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

#include "stacktrace.h"

std::deque<size_t> StackTrace::make_trace(
    const std::pair<size_t, StackTree::vertex_descriptor>& data) const {
  std::deque<size_t> this_stack;

  StackTree::vertex_descriptor act_item = data.second;

  this_stack.push_front(data.first);
  while ((act_item != root_node)) {
    this_stack.push_front(local_stack[act_item].addr);
    // we are not root, hence we have edges per-definition
    const auto edge = boost::out_edges(act_item, local_stack);
    act_item = boost::target(*(edge.first), local_stack);
  }

  return this_stack;
}

void StackTrace::clean() {
// currently disabled as way to expensive
// and does not work with vector-lists
#if 0
    bool delete_flag, sth_was_deleted;
    do {
        sth_was_deleted = false;

        for (auto it = local_stack.m_vertices.begin(); it != local_stack.m_vertices.end(); it++) {
            //if and only if the vertex is not  the current element, and it has no in_edges must be a top of stack
            if (*it != curr_elem && boost::in_degree(*it, local_stack) == 0) {
                delete_flag = true;
                for (auto jt = read_write.begin(); jt != read_write.end(); jt++) {//step through the read_write list to make sure no element is refering to it
                    if (jt->second.second == *it) {
                        delete_flag = false;
                        break;
                    }
                }
                if (delete_flag) {
                    boost::clear_vertex(*it, local_stack);
                    boost::remove_vertex(*it, local_stack);
                    sth_was_deleted = true;
                    break;
                }
            }
        }
    } while (sth_was_deleted);
#endif
}

void StackTrace::remove_function_element() {
  // std::lock_guard<ipc::spinlock> lg(lock);
  auto edge = boost::out_edges(curr_elem, local_stack);
  curr_elem = (boost::target(*(edge.first), local_stack));

#if 0
    pop_count++;
    if(pop_count > 1000){ // TODO: magic value?
        pop_count = 0;
        // clean has a huge performance impact
        clean();
    }
#endif
}

void StackTrace::insert_function_element(size_t element) {
  // std::lock_guard<ipc::spinlock> lg(lock);
  StackTree::vertex_descriptor tmp;

  if (boost::in_degree(curr_elem, local_stack) > 0) {
    auto in_edges = boost::in_edges(curr_elem, local_stack);
    for (auto it = in_edges.first; it != in_edges.second; ++it) {
      // check here if such a node is already existant and use it if so
      tmp = boost::source(*it, local_stack);
      size_t desc = local_stack[tmp].addr;

      if (element == desc) {
        curr_elem = tmp;  // node is already there, use it
        return;
      }
    }
  }

  tmp = boost::add_vertex({element}, local_stack);
  boost::add_edge(tmp, curr_elem,
                  local_stack);  // add an edge from last function to
                                 // this new call(they are stacked)
  curr_elem =
      tmp;  // make the current element of the graph the new function call;
}

StackTrace::StackTree::vertex_descriptor StackTrace::get_current_element()
    const {
  return curr_elem;
}
