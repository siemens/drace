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
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using pointer = T*;
  using const_pointer = const T*;
  using void_pointer = void*;
  using const_void_pointer = const void*;

  PoolAllocator<T, numChunks>() = default;
  ~PoolAllocator<T, numChunks>() = default;

  size_type max_size() const { return std::numeric_limits<size_type>::max(); }

  static pointer allocate() {
    return reinterpret_cast<pointer>(mem_pool.allocate());
  }

  static void deallocate(void* ptr) { mem_pool.deallocate(ptr); }

  static void usedMemory() { mem_pool.printUsedMemory(); }

  template <class U>
  PoolAllocator(const PoolAllocator<U, numChunks>& other) {}

  template <typename U>
  struct rebind {
    using other = PoolAllocator<U, numChunks>;
  };

 private:
  static MemoryPool mem_pool;
};
template <typename T, size_t numChunks>
MemoryPool PoolAllocator<T, numChunks>::mem_pool(sizeof(T), numChunks);

template <size_t size, size_t numChunks = 512>
class SizePoolAllocator {
 public:
  SizePoolAllocator<size, numChunks>() = default;
  ~SizePoolAllocator<size, numChunks>() = default;
  using size_type = size_t;

  size_type max_size() const { return std::numeric_limits<size_type>::max(); }

  static void* allocate() {
    return reinterpret_cast<void*>(mem_pool.allocate());
  }

  static void deallocate(void* ptr) { mem_pool.deallocate(ptr); }

  static void usedMemory() { mem_pool.printUsedMemory(); }

 private:
  static MemoryPool mem_pool;
};
template <size_t size, size_t numChunks>
MemoryPool SizePoolAllocator<size, numChunks>::mem_pool(size, numChunks);

#endif  //! POOL_ALLOCATOR_HEADER_H