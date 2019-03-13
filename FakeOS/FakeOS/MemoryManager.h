#pragma once

#include "Kernel.h"
#include <mutex>
#include <map>

class MemoryManager
{
public:
	MemoryManager();
	~MemoryManager();

	//allocate by page.
	//returning false indicates out of memory
	//the allocated memory may or may not be in memory
	bool virtualAllocate(/*IN*/ size_t size, /*OUT*/ int& startPage);

	//returning false indicates memory violation
	bool virtualFree(size_t startPage);

	//may cause page fault, and it's the only place that can cause page fault
	bool accessMemory(size_t pageNumber);

private:
	size_t usedMemoryPages;
	size_t usedSwapAreaPages;

	mutable std::mutex _mutex;
	
	//data structure for LRU
	//...

};

