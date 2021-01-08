#ifndef MEMORY_POOL_HEADER_H
#define MEMORY_POOL_HEADER_H 1
#pragma once

/*
 * DRace, a dynamic data race detector
 *
 * Copyright 2020 Siemens AG
 *
 * Authors:
 *   Mihai Robescu <mihai-gabriel.robescu@siemens.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <iostream>

/**
 *------------------------------------------------------------------------------
 *
 * Header File implementing a the memory pool required for the pool allocator
 *
 *------------------------------------------------------------------------------
 */

struct Chunk {
  Chunk *next;  // pointer to the next Chunk, when chunk is free
};

// For each allocator there will be a separate instantion of the memory pool
class MemoryPool {
 private:
  /// pointer to the first free address
  Chunk *free_pointer = nullptr;

  /// number of chunks in a block
  size_t num_chunks;

  /// chunk size equivalent to sizeof(T) from template specialization
  size_t chunk_size;

  /// block size
  size_t block_size;

  /// holds how many chunks were allocated until now
  size_t chunks_allocated = 0;

 public:
  MemoryPool() = default;
  MemoryPool(size_t chunkSize) : chunk_size(chunkSize) {}
  MemoryPool(size_t chunkSize, size_t numChunks)
      : chunk_size(chunkSize), num_chunks(numChunks) {
    block_size = chunk_size * num_chunks;
    free_pointer = get_more_memory();
  }
  Chunk *allocate() { return do_allocation(); }
  void deallocate(void *ptr) { do_deallocation(ptr); }
  void print_used_memory() {
    std::cout << "Memory Allocated: " << chunks_allocated * chunk_size
              << std::endl;
  }

 private:
  Chunk *do_allocation();
  Chunk *get_more_memory();  // allocate 1 block of chunks
  void do_deallocation(void *ptr);
};
#endif  // !MEMORY_POOL_HEADER_H
