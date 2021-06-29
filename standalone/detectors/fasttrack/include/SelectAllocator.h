#ifndef SELECT_ALLOCATOR_HEADER_H
#define SELECT_ALLOCATOR_HEADER_H

#include "PoolAllocator.h"
#include "StackTreeNode.h"

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

#endif  //! SELECT_ALLOCATOR_HEADER_H
