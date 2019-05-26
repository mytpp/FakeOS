#pragma once

#include "MemoryManager.h"
#include <mutex>
#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>


class PagedMemoryManager : public MemoryManager
{
public:
	PagedMemoryManager();
	~PagedMemoryManager();

	//Allocate by page.
	//Returning false indicates out of memory/
	//If it returns true, startPage is the start page number of allocated memory.
	//The allocated memory may or may not be in memory.
	bool virtualAllocate(
		/*IN*/ std::shared_ptr<ScheduleQueue::PCB> pcb,
		/*IN*/ const size_t size, 
		/*OUT*/ size_t& startPage) override; //page number of process address space

	//startPage is the page number of process address space to free.
	//Returning false indicates memory violation.
	bool virtualFree(
		std::shared_ptr<ScheduleQueue::PCB> pcb, const size_t startPage) override;

	//may cause page fault, and it's the only place that can cause page fault
	bool accessMemory(
		std::shared_ptr<ScheduleQueue::PCB> pcb, const size_t pageNumber) override;


private:
	mutable std::mutex _mutex; // protect all data below
	
	struct PageInfoEntry
	{
		PageInfoEntry(size_t frame, size_t page, std::shared_ptr<ScheduleQueue::PCB> pcb)
			:frameNumber(frame), pageNumber(page), owner(pcb) { }

		size_t frameNumber;
		size_t pageNumber;  //in user address space
		std::shared_ptr<ScheduleQueue::PCB> owner;
	};

	//data structure for LRU

	//Sorted by recent used time.
	//The front is the least recently used, the back is the most recently used
	std::list<PageInfoEntry> _frames; 
	std::list<PageInfoEntry>::iterator _nextToAllocate;
	size_t _freeMemoryPagesNum;
	//map frame number to its location in _frames;
	std::unordered_map<size_t, std::list<PageInfoEntry>::iterator> _frameLocator;
	
	//size_t _freeSwapAreaPagesNum; //==_freeSwapAreaPages.size() at most time
	std::unordered_set<size_t> _freeSwapAreaPages;
};

