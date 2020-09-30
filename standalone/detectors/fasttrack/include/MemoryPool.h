#ifndef MEMORY_POOL_HEADER_H
#define MEMORY_POOL_HEADER_H 1

#include <assert.h>
#include <iostream>
#include "prof.h"

struct Chunk {
  Chunk *next;  // pointer to the next Chunk, when chunk is free
};

// For each allocator there will be a separate instantion of the memory pool
class MemoryPool {
 private:
  Chunk *_FreePointer = nullptr;  // pointer to the first free address
  size_t _numChunks;              // number of chunks in a block
  size_t _chunkSize;  // chunk size equivalent to sizeof(T) from template
                      // specializtion
  size_t _blockSize;  // block size

  size_t _chunks_allocated =
      0;  // will hold how much memory was allocated until now

 public:
  MemoryPool() = default;
  MemoryPool(size_t chunkSize) : _chunkSize(chunkSize) {}
  MemoryPool(size_t chunkSize, size_t numChunks)
      : _chunkSize(chunkSize), _numChunks(numChunks) {
    _blockSize = _chunkSize * _numChunks;
    _FreePointer = getMoreMemory();
  }
  Chunk *allocate() { return do_allocation(); }
  void deallocate(void *ptr) { do_deallocation(ptr); }
  void printUsedMemory() {
    std::cout << "Memory Allocated: " << _chunks_allocated * _chunkSize
              << std::endl;
  }

 private:
  Chunk *do_allocation();
  Chunk *getMoreMemory();  // allocate 1 block of chunks
  void do_deallocation(void *ptr);
};
#endif  // !MEMORY_POOL_HEADER_H
