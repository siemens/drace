#ifndef POOL_ALLOCATOR_HEADER_H
#define POOL_ALLOCATOR_HEADER_H

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
 * \class PoolAllocator.
 * \brief it used to allocate memory in chunks. It can be used in combination
 * with the \class Segregator to split on the sizes Header File implementing a
 *
 * \note this version is used with types. The \class SizePoolAllocator is used
 * to allocate chunks of predefined sizes.
 *
 * \note There is also a thread-safe version. This is currently only
 * experimental
 */
template <typename T, size_t num_chunks = 512>
class PoolAllocator {
 public:
  using size_type = size_t;
  using pointer = T*;
  using void_pointer = void*;

  PoolAllocator<T, num_chunks>() = default;
  ~PoolAllocator<T, num_chunks>() = default;

  size_type max_size() const { return std::numeric_limits<size_type>::max(); }

  static pointer allocate() {
    return reinterpret_cast<pointer>(mem_pool.allocate());
  }

  static void deallocate(void_pointer ptr) { mem_pool.deallocate(ptr); }

  void usedMemory() { mem_pool.print_used_memory(); }

  template <class U>
  PoolAllocator(const PoolAllocator<U, num_chunks>& other) {}

  template <typename U>
  struct rebind {
    using other = PoolAllocator<U, num_chunks>;
  };

 private:
  static MemoryPool mem_pool;
};
template <typename T, size_t num_chunks>
MemoryPool PoolAllocator<T, num_chunks>::mem_pool(sizeof(T), num_chunks);

template <size_t size, size_t num_chunks = 512>
class SizePoolAllocator {
 public:
  using size_type = size_t;
  using void_pointer = void*;

  SizePoolAllocator<size, num_chunks>() = default;
  ~SizePoolAllocator<size, num_chunks>() = default;

  size_type max_size() const { return std::numeric_limits<size_type>::max(); }

  static void_pointer allocate() {
    return reinterpret_cast<void_pointer>(mem_pool.allocate());
  }

  static void deallocate(void_pointer ptr) { mem_pool.deallocate(ptr); }

  static void usedMemory() { mem_pool.print_used_memory(); }

 private:
  static MemoryPool mem_pool;
};
template <size_t size, size_t num_chunks>
MemoryPool SizePoolAllocator<size, num_chunks>::mem_pool(size, num_chunks);

template <class T, int threshold, class SmallAllocator, class LargeAllocator>
class Segregator {
 public:
  using size_type = size_t;
  using pointer = T*;
  using void_pointer = void*;

  pointer allocate(size_type n) {
    size_type size = n * sizeof(T);
    if (size < threshold) {
      // return new (reinterpret_cast<void*>(SmallAllocator::allocate())) T();
      return reinterpret_cast<pointer>(SmallAllocator::allocate());
    } else {
      return reinterpret_cast<pointer>(LargeAllocator::allocate());
    }
  }
  void deallocate(pointer p, size_type n) noexcept {
    size_t size = n * sizeof(T);
    if (size < threshold) {
      return SmallAllocator::deallocate(reinterpret_cast<void_pointer>(p));
    } else {
      return LargeAllocator::deallocate(reinterpret_cast<void_pointer>(p));
    }
  }
  template <typename U>
  struct rebind {
    using other = Segregator<U, threshold, SmallAllocator, LargeAllocator>;
  };
};

/**
 * \note using \class Segregator, we can split the type of allocator needed
 * based on size.
 * \brief Numbers are chosen based on the powers of 2 e.g.
 * SizePoolAllocator<16>, whereas the limit is chosen as the next integer after
 * the size limit e.g. 17
 */
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

/**
 * \class ThreadSafePoolAllocator.
 * \brief thread-safe version of a pool allocator
 * \param num_chunks represents number of chunks in a block
 */
template <typename T, size_t num_chunks = 512>
class ThreadSafePoolAllocator {
 private:
  std::atomic<Chunk*> free_pointer{nullptr};    // pointer to the first free
  size_t chunk_size = sizeof(T);                // chunk size equivalent
  size_t block_size = num_chunks * chunk_size;  // block size
  size_t chunks_allocated = 0;  // how much memory was allocated until now

 public:
  using size_type = size_t;
  using pointer = T*;
  using void_pointer = void*;

  ThreadSafePoolAllocator<T, num_chunks>() = default;
  ~ThreadSafePoolAllocator<T, num_chunks>() = default;

  size_type max_size() const { return std::numeric_limits<size_type>::max(); }

  pointer allocate() {
    if (nullptr == free_pointer.load(std::memory_order_release)) {
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

  void deallocate(void_pointer ptr) {
    Chunk* c = reinterpret_cast<Chunk*>(ptr);
    c->next = free_pointer.load(std::memory_order_release);
    free_pointer.store(c, std::memory_order_acquire);
    chunks_allocated--;
  }
};

#endif  //! POOL_ALLOCATOR_HEADER_H
