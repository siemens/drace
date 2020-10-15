#ifndef PREFIXTREE_STACKDEPOT_HEADER_H
#define PREFIXTREE_STACKDEPOT_HEADER_H 1
#pragma once

#include <ipc/spinlock.h>
#include <parallel_hashmap/phmap_utils.h>  // minimal header providing phmap::HashState()
#include <deque>
#include "parallel_hashmap/phmap.h"

#include <PoolAllocator.h>

#include <iostream>
#include <memory>
#include <unordered_map>

// using Allocator =

template <class K, class V>
using phmap_small = phmap::parallel_node_hash_map<
    K, V, phmap::container_internal::hash_default_hash<K>,
    phmap::container_internal::hash_default_eq<K>,
    DRaceAllocator<std::pair<const K, V>>,
    6,  // number of SubMaps; change here affects
    phmap::NullMutex>;

typedef struct TrieNode {
  size_t pc = -1;
  TrieNode* parent = nullptr;
  phmap::node_hash_map<size_t, TrieNode> _childNodes;

  TrieNode() = default;
} TrieNode;

class TrieStackDepot {
  TrieNode* _curr_elem = nullptr;
 public:
  TrieNode* GetCurrentElement() { return _curr_elem; }

  void InsertFunction(size_t pc) {
    if (_curr_elem == nullptr) {
      // TODO: using new is slow => PoolAllocator
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
      // as there can be still slements pointing to it
      _curr_elem = nullptr;
      return;
    }

    _curr_elem = _curr_elem->parent;
  }

  std::deque<size_t> MakeTrace(const std::pair<size_t, TrieNode*>& data) const {
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
