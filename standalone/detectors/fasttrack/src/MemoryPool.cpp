#include "MemoryPool.h"

Chunk* MemoryPool::do_allocation() {
  if (free_pointer == nullptr) {
    free_pointer = get_more_memory();
  }
  // now we can for sure allocate all the objects.
  Chunk* allocated = free_pointer;
  free_pointer = free_pointer->next;
  chunks_allocated++;
  return allocated;
}

Chunk* MemoryPool::get_more_memory() {
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

void MemoryPool::do_deallocation(void* ptr) {
  Chunk* c = reinterpret_cast<Chunk*>(ptr);
  c->next = free_pointer;
  free_pointer = c;
  chunks_allocated--;
}