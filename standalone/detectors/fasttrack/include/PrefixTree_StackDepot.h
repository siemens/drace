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

typedef struct TrieNode {
  size_t pc = -1;
  TrieNode* parent = nullptr;

  size_t child_count = 0;

  bool operator==(const TrieNode& o) const {
    return pc == o.pc && parent == o.parent;
  }

  TrieNode() {
    pc = -1;
    parent = nullptr;
  }
} TrieNode;

namespace std {
template <>
struct hash<TrieNode> {
  std::size_t operator()(TrieNode const& p) const {
    return phmap::HashState().combine(0, p.pc);
  }
};
}  // namespace std

using Allocator = PoolAllocator<TrieNode, 512>;

class TrieStackDepot {
  TrieNode* m_curr_elem = nullptr;
  phmap::flat_hash_map<TrieNode, phmap::flat_hash_map<size_t, TrieNode*>>
      m_trieNodeMap;

 public:
  TrieNode* GetCurrentElement() { return m_curr_elem; }

  void InsertFunction(size_t pc) {
    if (m_curr_elem == nullptr) {
      // using new was slow => PoolAllocator
      m_curr_elem = Allocator::allocate();
      m_curr_elem->parent = nullptr;
      m_curr_elem->pc = pc;
      auto it = m_trieNodeMap.find(*m_curr_elem);
      if (it == m_trieNodeMap.end()) {
        m_trieNodeMap.emplace_hint(it, *m_curr_elem,
                                   phmap::flat_hash_map<size_t, TrieNode*>());
      }
      return;
    }
    if (pc == m_curr_elem->pc) return;  // done for recursive functions;
    // no need to use more nodes for same function

    auto hash_it = m_trieNodeMap.find(*m_curr_elem);
    if (hash_it == m_trieNodeMap.end()) {
      hash_it = m_trieNodeMap.emplace_hint(
          hash_it, *m_curr_elem, phmap::flat_hash_map<size_t, TrieNode*>());
      hash_it->second.reserve(10);
    }

    auto it = hash_it->second.find((size_t)pc);
    if (it == hash_it->second.end()) {
      it = hash_it->second.emplace_hint(it, pc, Allocator::allocate());
      m_curr_elem->child_count++;
    }
    TrieNode* next = (it->second);
    next->parent = m_curr_elem;
    m_curr_elem = next;
    m_curr_elem->pc = pc;
  }

  void ExitFunction() {
    if (m_curr_elem == nullptr) return;  // func_exit before func_enter

    auto hash_it = m_trieNodeMap.find(*m_curr_elem);  // must be there
    if (m_curr_elem->parent == nullptr) {  // exiting the root function
      for (auto it = hash_it->second.begin(); it != hash_it->second.end();
           it++) {
        Allocator::deallocate(it->second);
      }
      Allocator::deallocate(m_curr_elem);
      m_curr_elem = nullptr;
      m_trieNodeMap.clear();
      return;
    }

    m_curr_elem = m_curr_elem->parent;
  }

  void PrintProf() {
    TrieNode* iter = m_curr_elem;
    while (iter->parent != nullptr) {
      iter = iter->parent;
    }

    // iter should be root now;
    auto hash_it = m_trieNodeMap.find(*iter);
    for (auto& x : hash_it->second) {
      std::cout << "child_count= " << x.second->child_count << " ";
      Print(x.second);
    }
  }

  void Print(TrieNode* root){
    auto hash_it = m_trieNodeMap.find(*root);
    for (auto& x : hash_it->second) {
      std::cout << "child_count= " << x.second->child_count << " ";
      Print(x.second);
    }
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
