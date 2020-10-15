#ifndef POOL_ALLOCATOR_HEADER_H
#define POOL_ALLOCATOR_HEADER_H 1
#pragma once

#include <iostream>
#include <limits>

#include <ipc/spinlock.h>
#include "MemoryPool.h"
#include "prof.h"

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
    using other =
        Segregator<U, threshold, SmallAllocator, LargeAllocator>;
  };
};

template <typename T, size_t numChunks = 512>
class PoolAllocator {
  // ipc::spinlock g_PoolAllocatorLock;

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
    // std::lock_guard<ipc::spinlock> lg(g_PoolAllocatorLock);
    return reinterpret_cast<pointer>(mem_pool.allocate());
  }

  static void deallocate(void* ptr) {
    // std::lock_guard<ipc::spinlock> lg(g_PoolAllocatorLock);
    mem_pool.deallocate(ptr);
  }

  void usedMemory() { mem_pool.printUsedMemory(); }

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

// struct Chunk {
//   Chunk* next;  // pointer to the next Chunk, when chunk is free
// };

// template <typename T, size_t numChunks = 512>
// class ThreadSafePoolAllocator {
//  private:
//   ipc::spinlock g_PoolAllocatorLock;
//   std::atomic<Chunk*> _FreePointer {nullptr};// pointer to the first free
//   address size_t _numChunks = numChunks;  // number of chunks in a block
//   size_t _chunkSize = sizeof(T);  // chunk size equivalent to sizeof(T) from
//                                   // template specializtion
//   size_t _blockSize = _numChunks * _chunkSize;  // block size

//   size_t _chunks_allocated =
//       0;  // will hold how much memory was allocated until now

//  public:
//   using value_type = T;
//   using size_type = size_t;
//   using difference_type = ptrdiff_t;
//   using pointer = T*;
//   using const_pointer = const T*;
//   using void_pointer = void*;
//   using const_void_pointer = const void*;

//   ThreadSafePoolAllocator<T, numChunks>() = default;
//   ~ThreadSafePoolAllocator<T, numChunks>() = default;

//   size_type max_size() const { return std::numeric_limits<size_type>::max();
//   }

//   pointer allocate() {
//     PRINT_TID();
//     DEB_FUNCTION();  // REMOVE_ME
//     SLEEP_THREAD();

//     // std::lock_guard<ipc::spinlock> lg(g_memPoolLock);

//     if (_FreePointer.load(std::memory_order_release) == nullptr) {
//       // std::lock_guard<ipc::spinlock> lg(g_PoolAllocatorLock);
//       _FreePointer.store(getMoreMemory(), std::memory_order_acquire);
//     }
//     // now we can for sure allocate all the objects.
//     Chunk* allocated = _FreePointer.load(std::memory_order_release);
//     _FreePointer.store(_FreePointer.load(std::memory_order_release),
//                        std::memory_order_relaxed);
//     _chunks_allocated++;
//     return reinterpret_cast<pointer>(allocated);
//   }

//   Chunk* getMoreMemory() {
//     PRINT_TID();
//     DEB_FUNCTION();  // REMOVE_ME
//     SLEEP_THREAD();

//     Chunk* start = reinterpret_cast<Chunk*>(operator new(_blockSize));
//     Chunk* it = start;
//     for (size_t i = 0; i < _numChunks - 1; ++i) {
//       it->next =
//           reinterpret_cast<Chunk*>(reinterpret_cast<char*>(it) + _chunkSize);
//       it = it->next;
//     }
//     it->next = nullptr;
//     return start;
//   }

//   void deallocate(void* ptr) {
//     PRINT_TID();
//     DEB_FUNCTION();  // REMOVE_ME
//     SLEEP_THREAD();

//     Chunk* c = reinterpret_cast<Chunk*>(ptr);
//     c->next = _FreePointer.load(std::memory_order_release);
//     _FreePointer.store(c, std::memory_order_acquire);
//     _chunks_allocated--;
//   }
// };

#endif  //! POOL_ALLOCATOR_HEADER_H