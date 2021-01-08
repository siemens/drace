#ifndef PREFIXTREE_STACKDEPOT_HEADER_H
#define PREFIXTREE_STACKDEPOT_HEADER_H 1
#pragma once

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

#include <ipc/spinlock.h>
#include <deque>
#include <iostream>
#include <memory>
#include "PoolAllocator.h"
#include "parallel_hashmap/phmap.h"

/**
 *------------------------------------------------------------------------------
 *
 * Header File that provides the initial implementation of a Prefix
 * Tree for being able to store call stacks. Check "PrefixTreeDepot.h" for a
 * cache efficient implementation
 *
 *------------------------------------------------------------------------------
 */

typedef struct TrieNode {
  size_t pc = -1;
  TrieNode* parent = nullptr;
  phmap::node_hash_map<size_t, TrieNode> _childNodes;

  TrieNode() = default;
} TrieNode;

class TrieStackDepot {
  TrieNode* _curr_elem = nullptr;

 public:
  TrieNode* get_current_element() { return _curr_elem; }

  void InsertFunction(size_t pc) {
    if (_curr_elem == nullptr) {
      // using new is slow => PoolAllocator in "PrefixTreeDepot.h"
      _curr_elem = new TrieNode();
      _curr_elem->parent = nullptr;
      _curr_elem->pc = pc;
    }
    if (pc == _curr_elem->pc) return;  // done for recursive functions;
    // no need to use more nodes for same function

    auto it = _curr_elem->_childNodes.find((size_t)pc);
    if (it == _curr_elem->_childNodes.end()) {
      it = _curr_elem->_childNodes.emplace_hint(it, pc, TrieNode());
    }

    TrieNode* next = &(it->second);
    next->parent = _curr_elem;
    _curr_elem = next;
    _curr_elem->pc = pc;
  }

  void ExitFunction() {
    if (_curr_elem == nullptr) return;  // func_exit before func_enter

    if (_curr_elem->parent == nullptr) {  // exiting the root function
      // delete _curr_elem; !! MEMORY IS NEVER FREED DOING THIS
      // as there can be still slements pointing to it;
      // TODO: remove all elements pointing to it as well
      _curr_elem = nullptr;
      return;
    }
    _curr_elem = _curr_elem->parent;
  }

  std::deque<size_t> make_trace(
      const std::pair<size_t, TrieNode*>& data) const {
    std::deque<size_t> this_stack;
    this_stack.emplace_front(data.first);

    TrieNode* iter = data.second;
    while (iter != nullptr) {
      this_stack.emplace_front(iter->pc);
      iter = iter->parent;
    }
    return std::move(this_stack);
  }
};
#endif
