#include "MemoryPool.h"

Chunk* MemoryPool::do_allocation() {
  // TODO: allocate more objects at once
  if (m_FreePointer == nullptr) {
    m_FreePointer = getMoreMemory();
  }
  // now we can for sure allocate all the objects.
  Chunk* allocated = m_FreePointer;
  m_FreePointer = m_FreePointer->next;
  m_chunks_allocated++;
  return allocated;
}

Chunk* MemoryPool::getMoreMemory() {
  Chunk* start = reinterpret_cast<Chunk*>(operator new(m_blockSize));
  Chunk* it = start;
  for (size_t i = 0; i < m_numChunks - 1; ++i) {
    it->next =
        reinterpret_cast<Chunk*>(reinterpret_cast<char*>(it) + m_chunkSize);
    it = it->next;
  }
  it->next = nullptr;
  // m_chunks_free += m_numChunks;
  return start;
}

void MemoryPool::do_deallocation(void* ptr) {
  Chunk* c = reinterpret_cast<Chunk*>(ptr);
  c->next = m_FreePointer;
  m_FreePointer = c;
  m_chunks_allocated--;
}