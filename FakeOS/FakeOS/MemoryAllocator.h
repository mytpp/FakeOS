#pragma once

#include "Kernel.h"
#include <mutex>
#include <map>

class MemoryAllocator
{
public:


	MemoryAllocator();
	~MemoryAllocator();

	//allocate by page.
	//return value indicates whether it succeeds or not
	int virtualAllocate(/*IN*/ size_t size, /*OUT*/ int& pageNum, /*OUT*/ bool inMemory);

	//maybe not safe, what if a process frees memory randomly?
	void virtualFree(size_t startAddress);

private:
	struct Chunk
	{
		size_t startAddress;
		size_t size;
		uint16_t pid;
		bool loaded;
	};

	//map startAddress to address chunk descriptor
	std::map<size_t, Chunk> allocatedMemory; //virtual memory

	mutable std::mutex _mutex;
	
	//data structure for LRU
	//...

};

