#ifndef TREEDEPOT_HEADER_H
#define TREEDEPOT_HEADER_H 1
#pragma once

#include <memory>
#include <vector>

#include "PoolAllocator.h"
#include "prof.h"

class INode{ 

  virtual iterator begin() = 0;

  virtual ~INode() = 0;
};

template<size_t N>
class Node : public virtual INode{
 public:
  static constexpr int MAX_SIZE = 10000;
  size_t pc = -1; //8
  Node* parent = nullptr;//8
  std::array<size_t, N> child_values;
  std::array<INode*, N> child_nodes;
  // std::vector<size_t, VectorAllocator<size_t>> child_values;
  // std::vector<Node*, VectorAllocator<Node*>> child_nodes;

  explicit Node() {
    child_values.reserve(MAX_SIZE);
    child_nodes.reserve(MAX_SIZE);
    pc = -1;
    parent = nullptr;
    for (int i = 0; i < MAX_SIZE; ++i) {
      child_values.push_back(-1);
      child_nodes.push_back(nullptr);
    }
  }

  explicit Node(size_t size) {
    pc = -1;
    parent = nullptr;
    child_values.reserve(size);
    child_nodes.reserve(size);
    for (int i = 0; i < size; ++i) {
      child_values.push_back(-1);
      child_nodes.push_back(nullptr);
    }
  }

  Node(const Node& other) = delete;
  ~Node()=default;

  Node& operator=(const Node& other) {
    pc = other.pc;
    parent = other.parent;
    for (int i = 0; i < other.child_values.size(); ++i) {
      child_values[i] = other.child_values[i];
      child_nodes[i] = other.child_nodes[i];
    }
    return *this;
  }
};

template <class T>
class Segregator {
  static constexpr int threshold1 = 3;
  using Allocator1 = SizePoolAllocator<64, 4096>;
  static constexpr int threshold2 = 7;
  using Allocator2 = SizePoolAllocator<128, 1024>;
  static constexpr int threshold3 = 11;
  using Allocator3 = SizePoolAllocator<192, 512>;
  static constexpr int threshold4 = 39;
  using Allocator4 = SizePoolAllocator<640, 256>;
  static constexpr int threshold5 = 199;
  using Allocator5 = SizePoolAllocator<199, 16>;
  using LargeAllocator = std::allocator<Node>;

 public:
  static T* allocate(size_t size) {
    if (size < threshold1) {
      return new (Allocator1::allocate()) T(threshold1);
    } else if (size < threshold2) {
      return new (Allocator2::allocate()) T(threshold2);
    } else if (size < threshold3) {
      return new (Allocator3::allocate()) T(threshold3);
    } else if (size < threshold4) {
      return new (Allocator4::allocate()) T(threshold4);
    } else if (size < threshold5) {
      return new (Allocator5::allocate()) T(threshold5);
    } else {  // allocate just 1;
      LargeAllocator al;
      T* new_t = std::allocator_traits<LargeAllocator>::allocate(al, 1);
      std::allocator_traits<LargeAllocator>::construct(al, new_t);
      return new_t;
    }
  }

  static void deallocate(Node* ptr, size_t size) {  // put Node* for POC
  //call destructor and deallocate
    if (size < threshold1) {
      // ptr->~Node();
      Allocator1::deallocate(ptr);
    } else if (size < threshold2) {
      // ptr->~Node();
      Allocator2::deallocate(ptr);
    } else if (size < threshold3) {
      // ptr->~Node();
      Allocator3::deallocate(ptr);
    } else if (size < threshold4) {
      // ptr->~Node();
      Allocator4::deallocate(ptr);
    } else if (size < threshold5) {
      // ptr->~Node();
      Allocator5::deallocate(ptr);
    } else {  // deallocate just 1;
      LargeAllocator al;
      // std::allocator_traits<LargeAllocator>::deallocate(al, ptr, 1);
      std::allocator_traits<LargeAllocator>::destroy(al, ptr);
      std::allocator_traits<LargeAllocator>::deallocate(al, ptr, 1);
    }
  }
};

using Allocator = Segregator<Node>;

class TreeDepot {
  Node* _curr_elem = nullptr;

 public:
  Node* GetCurrentElement() { return _curr_elem; }

  void InsertFunction(size_t pc) {
    // DEB_FUNCTION();
    if (_curr_elem == nullptr) {
      // the root function has to be called with a big size
      _curr_elem = Allocator::allocate(32);
      _curr_elem->parent = nullptr;
      _curr_elem->pc = pc;
      return;
    }

    if (pc == _curr_elem->pc) return;  // done for recursive functions;
    // no need to use more nodes for same function

    Node* next = nullptr;
    size_t size = _curr_elem->child_values.size();
    for (int i = 0; i < size; ++i) {
      if (_curr_elem->child_values[i] == pc) {
        next = _curr_elem->child_nodes[i];
        // if it is already there use it
        _curr_elem = next;
        return;
      }
    }

    // it is not the current node or any of the child nodes
    next = Allocator::allocate(1);
    next->pc = pc;
    next->parent = _curr_elem;
    for (int i = 0; i < size; ++i) {
      if (_curr_elem->child_values[i] == -1) {
        _curr_elem->child_values[i] = pc;
        _curr_elem->child_nodes[i] = next;
        _curr_elem = next;
        return;
      }
    }

    // If we got to here, it means that the current node should be of a bigger
    // size
    // => allocate next big thing;
    Node* tmp = Allocator::allocate(size);
    *tmp = *_curr_elem;  // might be really slow, use move instead of copy
                         // assignment operator
    Node* parent = _curr_elem->parent;
    if (parent) {
      size_t parent_size = parent->child_nodes.size();
      for (int i = 0; i < size; ++i) {
        if (parent->child_nodes[i] == _curr_elem) {
          parent->child_nodes[i] = tmp;
          break;
        }
      }  // replace new pointer in the parent list
    }

    for (int i = 0; i < size; ++i) {
      _curr_elem->child_nodes[i]->parent = tmp;
    }  // replace so that children of current node point to the new value

//deallocate also vectors within => it is done via the destructor
    Allocator::deallocate(_curr_elem, size - 1);
    _curr_elem = tmp;
    next->parent = _curr_elem;
    _curr_elem->child_values[size] = pc;
    _curr_elem->child_nodes[size] = next;
    _curr_elem = next;
  }

  void ExitFunction() {
    // DEB_FUNCTION();
    if (_curr_elem == nullptr) return;  // func_exit before func_enter

    if (_curr_elem->parent == nullptr) {  // exiting the root function
      Allocator::deallocate(_curr_elem, 32);
      _curr_elem = nullptr;
      return;
    }

    _curr_elem = _curr_elem->parent;
  }

  std::deque<size_t> MakeTrace(const std::pair<size_t, Node*>& data) const {
    std::deque<size_t> this_stack;
    this_stack.emplace_front(data.first);

    Node* iter = data.second;
    while (iter != nullptr) {
      this_stack.emplace_front(iter->pc);
      iter = iter->parent;
    }
    return std::move(this_stack);
  }
};

#endif