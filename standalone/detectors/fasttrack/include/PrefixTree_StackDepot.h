#ifndef PREFIXTREE_STACKDEPOT_HEADER_H
#define PREFIXTREE_STACKDEPOT_HEADER_H

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
 * \note Header File that provides the initial implementation of a Prefix
 * Tree for being able to store call stacks. Check "PrefixTreeDepot.h" for a
 * cache efficient implementation. using new is slow => PoolAllocator with
 * predefined sizes in "PrefixTreeDepot.h"
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

  void insert_function(size_t pc) {
    if (nullptr == _curr_elem) {
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

  void exit_function() {
    if (nullptr == _curr_elem) return;  // func_exit before func_enter

    if (nullptr == _curr_elem->parent) {  // exiting the root function
      // TODO: switch  TrieNode* _curr_elem  to std::shared_pointer to
      // deallocate memory once nothing points to it anymore
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
    while (nullptr != iter) {
      this_stack.emplace_front(iter->pc);
      iter = iter->parent;
    }
    return std::move(this_stack);
  }
};
#endif  // !PREFIXTREE_STACKDEPOT_HEADER_H
