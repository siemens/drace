#ifndef POOL_ALLOCATOR_HEADER_H
#define POOL_ALLOCATOR_HEADER_H 1
#pragma once

#include <iostream>
#include <limits>

#include "MemoryPool.h"

template <typename T, size_t numChunks = 512>
class PoolAllocator {
 public:
  using value_type = T;
  using size_type = size_t;  // size of a memory adress (a pointer)
  using difference_type = ptrdiff_t;
  using pointer = T*;
  using const_pointer = const T*;
  using void_pointer = void*;
  using const_void_pointer = const void*;

  PoolAllocator<T, numChunks>() = default;
  ~PoolAllocator <T, numChunks>() = default;

  // Allocator<T, numChunks>() { // this in case I wanna change the usage later.
  //	mem_pool.changeParam	s(sizeof(T), numChunks);
  //}

  // static pointer allocate(size_type numObjects) { //n represents the number
  // of Objects; OFFICIAL VERSION 	return
  //reinterpret_cast<pointer>(mem_pool.allocate(numObjects));
  //}

  static pointer allocate(
      size_type numObjects,
      const_void_pointer
          local_pointer) {  // takes also a locality hint, a pointer to a
                            // previous element allocated earlier
    return allocate(numObjects);
  }

  size_type max_size() const {  // maximum number of objects our allocator can
                                // allocate -> sometimes REQUIRED
    return std::numeric_limits<size_type>::max();
  }

  // static void deallocate(pointer ptr, size_type numObjects) { //OFFICIAL
  // VERSION 	mem_pool.deallocate(ptr, numObjects);
  //}

  static pointer allocate() {
    return reinterpret_cast<pointer>(mem_pool.allocate());
  }

  static void deallocate(pointer ptr) {  // OFFICIAL VERSION
    mem_pool.deallocate(ptr);
  }

  static void usedMemory() { mem_pool.printUsedMemory(); }

  template <class U>
  PoolAllocator(const PoolAllocator<U, numChunks>& other) {}

  template <typename U>
  struct rebind {
    using other = PoolAllocator<U, numChunks>;
  };

  // template<class U, class ... Args>
  // void construct(U* ptr, Args& ...) {
  //	new (ptr) (std::forward<Args>(args) ...);
  //}

  // pointer getAllocPointer() {
  // 	return m_AllocPointer;
  // }
 private:
  static MemoryPool
      mem_pool;  // static variables persist across specializations of templates
};
template <typename T, size_t numChunks>
MemoryPool PoolAllocator<T, numChunks>::mem_pool(sizeof(T), numChunks);
#endif  //! POOL_ALLOCATOR_HEADER_H