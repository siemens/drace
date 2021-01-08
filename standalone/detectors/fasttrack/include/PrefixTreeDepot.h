
#ifndef TREEDEPOT_HEADER_H
#define TREEDEPOT_HEADER_H 1
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

#include <deque>
#include <memory>
#include "PoolAllocator.h"

/**
 *------------------------------------------------------------------------------
 *
 * Header File that implements a prefix tree data structure to
 * store call stack elements. This is one is optimized to be
 * cache efficient, making each node a multiple of the size of the
 * cache line
 *
 *------------------------------------------------------------------------------
 */

class INode {
 public:
  size_t pc = -1;           // 8 bytes
  INode* parent = nullptr;  // 8 bytes

  virtual INode* fast_check(size_t pc) const {
    throw std::runtime_error("Not implemented");
    return nullptr;
  }

  virtual size_t size() const {
    throw std::runtime_error("Not implemented");
    return -1;
  }

  virtual ~INode() {}

  virtual bool add_child_node(INode* next, size_t pc) {
    throw std::runtime_error("Not implemented");
    return 1;
  }

  virtual void change_child_node(INode* tmp, INode* _curr_elem) {
    throw std::runtime_error("Not implemented");
  }

  virtual void change_parent_node(INode* tmp) {
    throw std::runtime_error("Not implemented");
  }
};

template <size_t N>
class Node : public INode {
 public:
  std::array<size_t, N> child_values;  // N * 8 bytes
  std::array<INode*, N> child_nodes;   // N * 8 bytes

  ~Node() = default;
  Node& operator=(const Node& other) = default;
  Node(const Node& other) = delete;

  explicit Node() {
    pc = -1;
    parent = nullptr;
    for (int i = 0; i < N; ++i) {
      child_values[i] = -1;
    }
    for (int i = 0; i < N; ++i) {
      child_nodes[i] = nullptr;
    }
  }

  size_t size() const final { return N; }

  INode* fast_check(size_t pc) const final {
    for (int i = 0; i < N; ++i) {
      if (child_values[i] == pc) {
        return child_nodes[i];
      }
    }
    return nullptr;
  }

  bool add_child_node(INode* next, size_t pc) final {
    for (int i = 0; i < N; ++i) {
      if (child_values[i] == -1) {
        child_values[i] = pc;
        child_nodes[i] = next;
        return true;
      }
    }
    return false;
  }

  void change_child_node(INode* tmp, INode* _curr_elem) final {
    for (int i = 0; i < N; ++i) {
      if (child_nodes[i] == _curr_elem) {
        child_nodes[i] = tmp;
        return;
      }
    }  // replace new pointer in the parent list
  }

  void change_parent_node(INode* tmp) final {
    for (int i = 0; i < N; ++i) {
      child_nodes[i]->parent = tmp;
    }
  }
};

template <class T>
class SelectAllocator {
 public:
  static constexpr int threshold1 = 2;
  using Allocator1 = PoolAllocator<Node<threshold1>, 8192>;
  static constexpr int threshold2 = 6;
  using Allocator2 = PoolAllocator<Node<threshold2>, 4096>;
  static constexpr int threshold3 = 10;
  using Allocator3 = PoolAllocator<Node<threshold3>, 64>;
  static constexpr int threshold4 = 38;
  using Allocator4 = PoolAllocator<Node<threshold4>, 64>;
  static constexpr int threshold5 = 198;
  using Allocator5 = PoolAllocator<Node<threshold5>, 32>;
  static constexpr int threshold6 = 1000;
  using LargeAllocator = std::allocator<Node<threshold6>>;

  Allocator1 al1;
  Allocator2 al2;
  Allocator3 al3;
  Allocator4 al4;
  Allocator5 al5;
  LargeAllocator alL;

  T* allocate(size_t size) {
    if (size < threshold1) {
      return new (reinterpret_cast<void*>(al1.allocate())) Node<threshold1>();
    } else if (size < threshold2) {
      return new (reinterpret_cast<void*>(al2.allocate())) Node<threshold2>();
    } else if (size < threshold3) {
      return new (reinterpret_cast<void*>(al3.allocate())) Node<threshold3>();
    } else if (size < threshold4) {
      return new (reinterpret_cast<void*>(al4.allocate())) Node<threshold4>();
    } else if (size < threshold5) {
      return new (reinterpret_cast<void*>(al5.allocate())) Node<threshold5>();
    } else {  // allocate just 1;
      Node<threshold6>* new_t =
          std::allocator_traits<LargeAllocator>::allocate(alL, 1);
      std::allocator_traits<LargeAllocator>::construct(alL, new_t);
      return new_t;
    }
  }

  void deallocate(INode* ptr, size_t size) {
    if (size < threshold1) {
      // Node<threshold1>* tmp = dynamic_cast<Node<threshold1>*>(ptr);
      // tmp->~Node<threshold1>(); //doens't work when I am calling destructor
      al1.deallocate(ptr);
    } else if (size < threshold2) {
      al2.deallocate(ptr);
    } else if (size < threshold3) {
      al3.deallocate(ptr);
    } else if (size < threshold4) {
      al4.deallocate(ptr);
    } else if (size < threshold5) {
      al5.deallocate(ptr);
    } else {  // deallocate just 1;
      Node<threshold6>* tmp = dynamic_cast<Node<threshold6>*>(ptr);
      std::allocator_traits<LargeAllocator>::destroy(alL, tmp);
      std::allocator_traits<LargeAllocator>::deallocate(alL, tmp, 1);
    }
  }
};

using Allocator = SelectAllocator<INode>;
extern ipc::spinlock read_write_lock;

class TreeDepot {
  INode* _curr_elem = nullptr;
  Allocator al;

 public:
  INode* get_current_element() {
    // std::lock_guard<ipc::spinlock> lg(_read_write_lock);
    return _curr_elem;
  }

  void insert_function_element(size_t pc) {
    std::lock_guard<ipc::spinlock> lg(read_write_lock);

    if (_curr_elem == nullptr) {
      // the root function has to be called with a big size
      _curr_elem = al.allocate(5);
      _curr_elem->parent = nullptr;
      _curr_elem->pc = pc;
      return;
    }

    if (pc == _curr_elem->pc) return;  // done for recursive functions;
    // no need to use more nodes for same function

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
    // If we got to here, it means that the current node should be of a
    // bigger size => allocate next big thing;
    INode* tmp = al.allocate(_curr_elem->size());
    *tmp = *_curr_elem;  // might be really slow, use move instead of copy
                         // assignment operator
    INode* parent = _curr_elem->parent;
    if (parent) {
      parent->change_child_node(tmp, _curr_elem);
    }
    // replace so that children of current node point to the new value
    _curr_elem->change_parent_node(tmp);

    // TODO: MUST replace it too in ThreadState::read_write
    // Allocator::deallocate(_curr_elem, _curr_elem->size() - 1);

    _curr_elem = tmp;
    next->parent = _curr_elem;

    // we already know that here we can go to the end.
    _curr_elem->add_child_node(next, pc);
    _curr_elem = next;
  }

  void remove_function_element() {
    std::lock_guard<ipc::spinlock> lg(read_write_lock);

    if (_curr_elem == nullptr) return;  // func_exit before func_enter

    if (_curr_elem->parent == nullptr) {  // exiting the root function
      // Allocator::deallocate(_curr_elem, _curr_elem->size() - 1);
      // not deallocating anymore because node might be used
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
    while (iter != nullptr) {
      this_stack.emplace_front(iter->pc);
      iter = iter->parent;
    }
    return std::move(this_stack);
  }
};

#endif
