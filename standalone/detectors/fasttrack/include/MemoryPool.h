#ifndef MEMORY_POOL_HEADER_H
#define MEMORY_POOL_HEADER_H 1

#include <assert.h>
#include <iostream>
#include "prof.h"

struct Chunk
{
	Chunk *next; //pointer to the next Chunk, when chunk is free
};

//For each allocator there will be a separate instantion of the memory pool
class MemoryPool
{
private:
	Chunk *m_FreePointer = nullptr; // pointer to the first free address
	size_t m_numChunks;				//number of chunks in a block
	size_t m_chunkSize;				//chunk size equivalent to sizeof(T) from template specializtion
	size_t m_blockSize;				//block size

	size_t m_chunks_allocated = 0; //will hold how much memory was allocated until now
	size_t m_chunks_free;
public:
	MemoryPool() = default;
	MemoryPool(size_t chunkSize) : m_chunkSize(chunkSize) {}
	MemoryPool(size_t chunkSize, size_t numChunks) : m_chunkSize(chunkSize), m_numChunks(numChunks)
	{
		m_blockSize = m_chunkSize * m_numChunks;
		m_FreePointer = getMoreMemory();
	}
	Chunk *allocate() { return do_allocation(); }
	void deallocate(void *ptr) { do_deallocation(ptr); }
	void printUsedMemory() { std::cout << "Memory Allocated: " << m_chunks_allocated * m_chunkSize << std::endl; }

private:
	Chunk *do_allocation();
	Chunk *getMoreMemory(); //allocate 1 block of chunks (of memory)
	void do_deallocation(void *ptr);
};
#endif  // !MEMORY_POOL_HEADER_H
