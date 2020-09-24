#ifndef TREEDEPOT_HEADER_H
#define TREEDEPOT_HEADER_H 1
#pragma once

#include <memory>
#include <vector>

#include <PoolAllocator.h>

// cannot make it work with arrays
// template <int N>
// struct Node {
//   size_t pc = -1;
//   Node* parent = nullptr;
//   // using std::arrays I will get exactly the memory I need
//   std::array<size_t, N> child_values;
//   std::array<Node*, N> child_nodes;
// };

struct Node {
  size_t pc = -1;
  Node* parent = nullptr;
  std::vector<size_t> child_values;
  std::vector<Node*> child_nodes;

  Node(size_t size) : child_values(size), child_nodes(size) {
    pc = -1;
    parent = nullptr;
  }

  Node(const Node& other) {
    pc = other.pc;
    parent = other.parent;
    for (int i = 0; i < other.child_values.size(); ++i) {
      child_values[i] = other.child_values[i];
      child_nodes[i] = other.child_nodes[i];
    }
  }
};

template <class Allocator1, int threshold1, class Allocator2, int threshold2,
          class Allocator3, int threshold3, class Allocator4, int threshold4,
          class Allocator5, int threshold5, class LargeAllocator>
class Segregator {
 public:
  static void* allocate(size_t size) {
    if (size < threshold1) {
      return Allocator1::allocate();
    } else if (size < threshold2) {
      return Allocator2::allocate();
    } else if (size < threshold3) {
      return Allocator3::allocate();
    } else if (size < threshold4) {
      return Allocator4::allocate();
    } else if (size < threshold5) {
      return Allocator5::allocate();
    } else {  // allocate just 1;
      LargeAllocator al;
      return std::allocator_traits<LargeAllocator>::allocate(al, 1);
    }
  }

  static void deallocate(Node* ptr, size_t size) {  // put Node* for POC
    if (size < threshold1) {
      Allocator1::deallocate(ptr);
    } else if (size < threshold2) {
      Allocator2::deallocate(ptr);
    } else if (size < threshold3) {
      Allocator3::deallocate(ptr);
    } else if (size < threshold4) {
      Allocator4::deallocate(ptr);
    } else if (size < threshold5) {
      Allocator5::deallocate(ptr);
    } else {  // deallocate just 1;
      LargeAllocator al;
      std::allocator_traits<LargeAllocator>::deallocate(al, ptr, 1);
    }
  }
};

// using Allocator = PoolAllocator<Node, 1024>;
using Allocator =
    Segregator<SizePoolAllocator<64, 2048>, 3, SizePoolAllocator<128, 1024>, 7,
               SizePoolAllocator<192, 512>, 11, SizePoolAllocator<640, 256>, 39,
               SizePoolAllocator<3200, 16>, 199, std::allocator<Node>>;

class TreeDepot {
  Node* _curr_elem = nullptr;

 public:
  Node* GetCurrentElement() { return _curr_elem; }

  void InsertFunction(size_t pc) {
    if (_curr_elem == nullptr) {
      _curr_elem = new (Allocator::allocate(32)) Node(39);
      // the root function has to be called with a big size
      _curr_elem->parent = nullptr;
      _curr_elem->pc = pc;
    }

    if (pc == _curr_elem->pc) return;  // done for recursive functions;
    // no need to use more nodes for same function

    Node* next;
    size_t size = _curr_elem->child_values.size();
    for (int i = 0; i < size; ++i) {
      if (_curr_elem->child_values[i] == pc) {
        next = _curr_elem->child_nodes[i];
        break;
      }
    }

    if (next == nullptr) {
      // it is not the current node or any of the child nodes
      next = new (Allocator::allocate(1)) Node(3);
      next->pc = pc;
      next->parent = _curr_elem;
      int i = 0;
      for (; i < size; ++i) {
        if (_curr_elem->child_values[i] != -1) {
          _curr_elem->child_values[i] = pc;
          _curr_elem->child_nodes[i] = next;
          break;
        }
      }
      if (i == size) {
        // allocate next big thing;
        Node* tmp = new (Allocator::allocate(size)) Node(*_curr_elem);

        Node* parent = _curr_elem->parent;
        for (auto it = parent->child_nodes.begin();
             it != parent->child_nodes.end(); it++) {
          if (*it == _curr_elem) *it = tmp;
        }  // replace new pointer in the parent list

        Allocator::deallocate(_curr_elem, size - 1);
        _curr_elem = tmp;
        next->parent = _curr_elem;
        _curr_elem->child_values[i] = pc;
        _curr_elem->child_nodes[i] = next;
      }
    }
    _curr_elem = next;
  }

  void ExitFunction() {
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