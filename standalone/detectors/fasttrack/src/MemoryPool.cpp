#include "MemoryPool.h"

Chunk* MemoryPool::do_allocation() {
  if (_FreePointer == nullptr) {
    _FreePointer = getMoreMemory();
  }
  // now we can for sure allocate all the objects.
  Chunk* allocated = _FreePointer;
  _FreePointer = _FreePointer->next;
  _chunks_allocated++;
  return allocated;
}

Chunk* MemoryPool::getMoreMemory() {
  Chunk* start = reinterpret_cast<Chunk*>(operator new(_blockSize));
  Chunk* it = start;
  for (size_t i = 0; i < _numChunks - 1; ++i) {
    it->next =
        reinterpret_cast<Chunk*>(reinterpret_cast<char*>(it) + _chunkSize);
    it = it->next;
  }
  it->next = nullptr;
  return start;
}

void MemoryPool::do_deallocation(void* ptr) {
  Chunk* c = reinterpret_cast<Chunk*>(ptr);
  c->next = _FreePointer;
  _FreePointer = c;
  _chunks_allocated--;
}