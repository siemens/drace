#ifndef TREEDEPOT_HEADER_H
#define TREEDEPOT_HEADER_H

/*
 * DRace, a dynamic data race detector
 *
 * Copyright 2020 Siemens AG
 *
 * Authors:
 *   Mihai Robescu <mihai-gabriel.robescu@siemens.com>
 *   Felix Moessbauer <felix.moessbauer@siemens.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <deque>
#include <memory>

#include "SelectAllocator.h"
#include "StackTreeNode.h"

/**
 * \note Header File that implements a prefix tree data structure to
 * store call stack elements. This is one is optimized to be
 * cache efficient, making each node a multiple of the size of the
 * cache line
 */

using Allocator = SelectAllocator<INode>;
extern ipc::spinlock read_write_lock;

class TreeDepot {
  INode* _curr_elem = nullptr;
  Allocator al;

 public:
  INode* get_current_element() { return _curr_elem; }

  void insert_function_element(size_t pc) {
    std::lock_guard<ipc::spinlock> lg(read_write_lock);

    if (nullptr == _curr_elem) {
      // the root function has to be called with a big size
      _curr_elem = al.allocate(5);
      _curr_elem->parent = nullptr;
      _curr_elem->pc = pc;
      return;
    }

    /**
     * \note done for recursive functions; no need to use more nodes for same
     * function
     */
    if (pc == _curr_elem->pc) return;

    INode* next = _curr_elem->fast_check(pc);
    if (next) {
      _curr_elem = next;
      return;
    } else {  // it is not the current node or any of the child nodes
      next = al.allocate(1);
      next->pc = pc;
      next->parent = _curr_elem;
      if (_curr_elem->add_child_node(next, pc)) {
        _curr_elem = next;
        return;
      }
    }

    /**
     * \note If we got to here, it means that the current node should be of a
     * bigger size => allocate next big thing;
     */
    INode* tmp = al.allocate(_curr_elem->get_size());
    *tmp = *_curr_elem;
    INode* parent = _curr_elem->parent;
    if (parent) {
      parent->change_child_node(tmp, _curr_elem);
    }

    // replace so that children of current node point to the new value
    _curr_elem->change_parent_node(tmp);

    _curr_elem = tmp;
    next->parent = _curr_elem;

    // we already know that here we can go to the end.
    _curr_elem->add_child_node(next, pc);
    _curr_elem = next;
  }

  void remove_function_element() {
    std::lock_guard<ipc::spinlock> lg(read_write_lock);

    // func_exit before func_enter might happen. The instrumentation side might skip a func_enter call
    if (nullptr == _curr_elem) return; 

    if (nullptr == _curr_elem->parent) {  // exiting the root function
      // TODO: switch INode* to std::shared_pointer to deallocate memory once
      // nothing points to it anymore
      _curr_elem = nullptr;
      return;
    }
    _curr_elem = _curr_elem->parent;
  }

  std::deque<size_t> make_trace(const std::pair<size_t, INode*>& data) const {
    std::lock_guard<ipc::spinlock> lg(read_write_lock);

    std::deque<size_t> this_stack;
    this_stack.emplace_front(data.first);

    INode* iter = data.second;
    while (nullptr != iter) {
      this_stack.emplace_front(iter->pc);
      iter = iter->parent;
    }
    return std::move(this_stack);
  }
};

#endif  // !TREEDEPOT_HEADER_H
