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
  while ((act_item != _root)) {
    this_stack.push_front(_local_stack[act_item].addr);
    // we are not root, hence we have edges per-definition
    const auto edge = boost::out_edges(act_item, _local_stack);
    act_item = boost::target(*(edge.first), _local_stack);
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

        for (auto it = _local_stack.m_vertices.begin(); it != _local_stack.m_vertices.end(); it++) {
            //if and only if the vertex is not  the current element, and it has no in_edges must be a top of stack
            if (*it != _ce && boost::in_degree(*it, _local_stack) == 0) {
                delete_flag = true;
                for (auto jt = _read_write.begin(); jt != _read_write.end(); jt++) {//step through the read_write list to make sure no element is refering to it
                    if (jt->second.second == *it) {
                        delete_flag = false;
                        break;
                    }
                }
                if (delete_flag) {
                    boost::clear_vertex(*it, _local_stack);
                    boost::remove_vertex(*it, _local_stack);
                    sth_was_deleted = true;
                    break;
                }
            }
        }
    } while (sth_was_deleted);
#endif
}

void StackTrace::pop_stack_element() {
  std::lock_guard<ipc::spinlock> lg(lock);
  auto edge = boost::out_edges(_ce, _local_stack);
  _ce = (boost::target(*(edge.first), _local_stack));

#if 0
    _pop_count++;
    if(_pop_count > 1000){ // TODO: magic value?
        _pop_count = 0;
        // clean has a huge performance impact
        clean();
    }
#endif
}

void StackTrace::push_stack_element(size_t element) {
  std::lock_guard<ipc::spinlock> lg(lock);
  StackTree::vertex_descriptor tmp;

  if (boost::in_degree(_ce, _local_stack) > 0) {
    auto in_edges = boost::in_edges(_ce, _local_stack);
    for (auto it = in_edges.first; it != in_edges.second; ++it) {
      // check here if such a node is already existant and use it if so
      tmp = boost::source(*it, _local_stack);
      auto desc = _local_stack[tmp].addr;

      if (element == desc) {
        _ce = tmp;  // node is already there, use it
        return;
      }
    }
  }

  tmp = boost::add_vertex({element}, _local_stack);
  boost::add_edge(tmp, _ce, _local_stack);//add an edge from last function to this new call(they are stacked)
  _ce = tmp; //make the current element of the graph the new function call;
}

/// when a var is written or read, it copies the stack and adds the pc of the
/// r/w operation to be able to return the stack trace if a race was detected
void StackTrace::set_read_write(size_t addr, size_t pc) {

  std::lock_guard<ipc::spinlock> lg(lock);
  auto it = _read_write.find(addr);
  if (it == _read_write.end()) {
    _read_write.insert({addr, {pc, _ce}});
  } else {
    it->second = {pc, _ce};
  }
}

/// returns a stack trace of a clock for handing it over to drace
std::deque<size_t> StackTrace::return_stack_trace(size_t address) const {
  std::lock_guard<ipc::spinlock> lg(lock);
  auto it = _read_write.find(address);
  if (it != _read_write.end()) {
    auto data = it->second;
    return make_trace(data);
  }
  // A read/write operation was not tracked correctly => return empty stacktrace
  return {};
}
