#ifndef TREEDEPOT_HEADER_H
#define TREEDEPOT_HEADER_H 1
#pragma once

#include <memory>
#include <vector>

#include "PoolAllocator.h"
#include "prof.h"

class INode {
 public:
  size_t pc = -1;           // 8 bytes
  INode* parent = nullptr;  // 8 bytes

  virtual INode* FastCheck(size_t pc) const {
    throw std::runtime_error("Not implemented");
    return nullptr;
  }

  virtual size_t size() const {
    throw std::runtime_error("Not implemented");
    return -1;
  }

  virtual ~INode() {}

  virtual bool AddChildNode(INode* next, size_t pc) {
    throw std::runtime_error("Not implemented");
    return 1;
  }

  virtual void ChangeChildNode(INode* tmp, INode* _curr_elem) {
    throw std::runtime_error("Not implemented");
  }

  virtual void ChangeParentNode(INode* tmp) {
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

  size_t size() const override { return N; }

  INode* FastCheck(size_t pc) const override {
    for (int i = 0; i < N; ++i) {
      if (child_values[i] == pc) {
        return child_nodes[i];
      }
    }
    return nullptr;
  }

  bool AddChildNode(INode* next, size_t pc) override {
    for (int i = 0; i < N; ++i) {
      if (child_values[i] == -1) {
        child_values[i] = pc;
        child_nodes[i] = next;
        return true;
      }
    }
    return false;
  }

  void ChangeChildNode(INode* tmp, INode* _curr_elem) override {
    for (int i = 0; i < N; ++i) {
      if (child_nodes[i] == _curr_elem) {
        child_nodes[i] = tmp;
        return;
      }
    }  // replace new pointer in the parent list
  }

  void ChangeParentNode(INode* tmp) override {
    for (int i = 0; i < N; ++i) {
      child_nodes[i]->parent = tmp;
    }
  }
};

using Allocator = Segregator<
    Node<2>, 2, ThreadSafePoolAllocator<Node<2>, 4096>,
    Segregator<
        Node<6>, 6, ThreadSafePoolAllocator<Node<6>, 2048>,
        Segregator<
            Node<10>, 10, ThreadSafePoolAllocator<Node<10>, 512>,
            Segregator<Node<38>, 38, ThreadSafePoolAllocator<Node<38>, 256>,
                       Segregator<Node<198>, 198,
                                  ThreadSafePoolAllocator<Node<198>, 16>,
                                  Segregator<Node<1000>, 1000,
                                             std::allocator<Node<1000>>>>>>>>;

template <class T>
class Segregator {
 public:
  static constexpr int threshold1 = 2;
  using Allocator1 = ThreadSafePoolAllocator<Node<threshold1>, 4096>;
  static constexpr int threshold2 = 6;
  using Allocator2 = ThreadSafePoolAllocator<Node<threshold2>, 2048>;
  static constexpr int threshold3 = 10;
  using Allocator3 = ThreadSafePoolAllocator<Node<threshold3>, 512>;
  static constexpr int threshold4 = 38;
  using Allocator4 = ThreadSafePoolAllocator<Node<threshold4>, 256>;
  static constexpr int threshold5 = 198;
  using Allocator5 = ThreadSafePoolAllocator<Node<threshold5>, 16>;
  static constexpr int threshold6 = 1000;
  using LargeAllocator = std::allocator<Node<threshold6>>;

  Allocator1 al1;
  Allocator2 al2;
  Allocator3 al3;
  Allocator4 al4;
  Allocator5 al5;
  LargeAllocator alL;

  // template <class T, std::enable_if<Threshold == 3, void>::type* = nullptr>
  // static T* allocate(size_t size) {
  //   new (reinterpret_cast<void*>(Allocator1::allocate()))
  //       Node<threshold1>(threshold1);
  // }

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
      // LargeAllocator al;
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
      // LargeAllocator al;
      // std::allocator_traits<LargeAllocator>::deallocate(al, ptr, 1);
      Node<threshold6>* tmp = dynamic_cast<Node<threshold6>*>(ptr);
      std::allocator_traits<LargeAllocator>::destroy(alL, tmp);
      std::allocator_traits<LargeAllocator>::deallocate(alL, tmp, 1);
    }
  }
};

using Allocator = Segregator<INode>;
extern ipc::spinlock _read_write_lock;

class TreeDepot {
  INode* _curr_elem = nullptr;
  Allocator al;

 public:
  INode* GetCurrentElement() {
    // std::lock_guard<ipc::spinlock> lg(_read_write_lock);

    DEB_FUNCTION();  // REMOVE_ME
    if (_curr_elem) {
      PRINT_TID();
      newline();
      // SLEEP_THREAD();
      return _curr_elem;
    } else {
      PRINT_TID();
      printf("Not logical value at line number %d in file %s\n", __LINE__,
             __FILE__);
      return nullptr;
    }
  }

  void InsertFunction(size_t pc) {
    std::lock_guard<ipc::spinlock> lg(_read_write_lock);

    PRINT_TID();
    DEB_FUNCTION();  // REMOVE_ME

    if (_curr_elem == nullptr) {
      // the root function has to be called with a big size
      _curr_elem = al.allocate(5);
      _curr_elem->parent = nullptr;
      _curr_elem->pc = pc;
      return;
    }

    if (pc == _curr_elem->pc) return;  // done for recursive functions;
    // no need to use more nodes for same function

    INode* next = _curr_elem->FastCheck(pc);
    if (next) {
      _curr_elem = next;
      return;
    } else {  // it is not the current node or any of the child nodes
      next = al.allocate(1);
      next->pc = pc;
      next->parent = _curr_elem;
      if (_curr_elem->AddChildNode(next, pc)) {
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
      parent->ChangeChildNode(tmp, _curr_elem);
    }
    // replace so that children of current node point to the new value
    _curr_elem->ChangeParentNode(tmp);

    // TODO: MUST replace it too in ThreadState::_read_write
    // Allocator::deallocate(_curr_elem, _curr_elem->size() - 1);

    _curr_elem = tmp;
    next->parent = _curr_elem;

    // we already know that here we can go to the end.
    _curr_elem->AddChildNode(next, pc);
    _curr_elem = next;
  }

  void ExitFunction() {
    std::lock_guard<ipc::spinlock> lg(_read_write_lock);

    PRINT_TID();
    DEB_FUNCTION();  // REMOVE_ME

    if (_curr_elem == nullptr) return;  // func_exit before func_enter

    if (_curr_elem->parent == nullptr) {  // exiting the root function
      // Allocator::deallocate(_curr_elem, _curr_elem->size() - 1);
      // not deallocating anymore because node might be used
      _curr_elem = nullptr;
      return;
    }
    _curr_elem = _curr_elem->parent;
  }

  std::deque<size_t> MakeTrace(const std::pair<size_t, INode*>& data) const {
    DEB_FUNCTION();  // REMOVE_ME

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
