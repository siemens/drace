#ifndef POOL_ALLOCATOR_HEADER_H
#define POOL_ALLOCATOR_HEADER_H 1
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

#include <iostream>
#include <limits>

#include <ipc/spinlock.h>
#include "MemoryPool.h"

/**
 *------------------------------------------------------------------------------
 *
 * Header File implementing a pool allocator. It can also be used
 * in combination with the Segregator class, such as on line 136.
 *
 * There is also a thread-safe version. This is currently only
 * experimental
 *
 *------------------------------------------------------------------------------
 */

template <class T, int threshold, class SmallAllocator, class LargeAllocator>
class Segregator {
 public:
  using value_type = T;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using pointer = T*;
  using const_pointer = const T*;
  using void_pointer = void*;
  using const_void_pointer = const void*;

  T* allocate(size_t n) {
    size_t size = n * sizeof(T);
    if (size < threshold) {
      // return new (reinterpret_cast<void*>(SmallAllocator::allocate())) T();
      return reinterpret_cast<void*>(SmallAllocator::allocate());
    } else {
      return reinterpret_cast<void*>(LargeAllocator::allocate());
    }
  }
  void deallocate(T* p, std::size_t n) noexcept {
    size_t size = n * sizeof(T);
    if (size < threshold) {
      return SmallAllocator::deallocate(reinterpret_cast<void*>(p));
    } else {
      return LargeAllocator::deallocate(reinterpret_cast<void*>(p));
    }
  }
  template <typename U>
  struct rebind {
    using other = Segregator<U, threshold, SmallAllocator, LargeAllocator>;
  };
};

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

  static void deallocate(void* ptr) {
    mem_pool.deallocate(ptr);
  }

  void usedMemory() { mem_pool.print_used_memory(); }

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

  static void usedMemory() { mem_pool.print_used_memory(); }

 private:
  static MemoryPool mem_pool;
};
template <size_t size, size_t numChunks>
MemoryPool SizePoolAllocator<size, numChunks>::mem_pool(size, numChunks);

template <class T>
using DRaceAllocator = Segregator<
    T, 5, SizePoolAllocator<4>,
    Segregator<
        T, 9, SizePoolAllocator<8>,
        Segregator<
            T, 17, SizePoolAllocator<16>,
            Segregator<T, 65, SizePoolAllocator<64>,
                       Segregator<T, 257, SizePoolAllocator<256>,
                                  Segregator<T, 1025, SizePoolAllocator<1024>,
                                             std::allocator<T>>>>>>>;

template <typename T, size_t numChunks = 512>
class ThreadSafePoolAllocator {
 private:
  std::atomic<Chunk*> free_pointer{nullptr};    // pointer to the first free
  size_t num_chunks = numChunks;                // number of chunks in a block
  size_t chunk_size = sizeof(T);                // chunk size equivalent
  size_t block_size = num_chunks * chunk_size;  // block size
  size_t chunks_allocated = 0;  // how much memory was allocated until now

 public:
  using value_type = T;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using pointer = T*;
  using const_pointer = const T*;
  using void_pointer = void*;
  using const_void_pointer = const void*;

  ThreadSafePoolAllocator<T, numChunks>() = default;
  ~ThreadSafePoolAllocator<T, numChunks>() = default;

  size_type max_size() const { return std::numeric_limits<size_type>::max(); }

  pointer allocate() {
    if (free_pointer.load(std::memory_order_release) == nullptr) {
      free_pointer.store(get_more_memory(), std::memory_order_acquire);
    }
    // now we can for sure allocate all the objects.
    Chunk* allocated = free_pointer.load(std::memory_order_release);
    free_pointer.store(free_pointer.load(std::memory_order_release),
                       std::memory_order_acquire);
    chunks_allocated++;
    return reinterpret_cast<pointer>(allocated);
  }

  Chunk* get_more_memory() {
    Chunk* start = reinterpret_cast<Chunk*>(operator new(block_size));
    Chunk* it = start;
    for (size_t i = 0; i < num_chunks - 1; ++i) {
      it->next =
          reinterpret_cast<Chunk*>(reinterpret_cast<char*>(it) + chunk_size);
      it = it->next;
    }
    it->next = nullptr;
    return start;
  }

  void deallocate(void* ptr) {
    Chunk* c = reinterpret_cast<Chunk*>(ptr);
    c->next = free_pointer.load(std::memory_order_release);
    free_pointer.store(c, std::memory_order_acquire);
    chunks_allocated--;
  }
};

#endif  //! POOL_ALLOCATOR_HEADER_H