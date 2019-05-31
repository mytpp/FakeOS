#pragma once

#include "MemoryManager.h"
#include <mutex>
#include <vector>
#include <list>
#include <array>
#include <unordered_map>
#include <unordered_set>


class SegmentedMemoryManager : public MemoryManager
{
public:
	SegmentedMemoryManager();
	~SegmentedMemoryManager();

	//Allocate by segment.
	//Returning false indicates out of memory/
	//If it returns true, firt 4 bits of startPage is the start segment number of allocated memory, left is the offset of the segment 
	//The allocated memory may or may not be in memory.
	bool virtualAllocate(
		/*IN*/ std::shared_ptr<ScheduleQueue::PCB> pcb,
		/*IN*/ const size_t size,
		/*OUT*/ size_t& start) override; //when 'start' as an input, means the the Segmentnumber which demand memory

//startPage is the page number of process address space to free.
//Returning false indicates memory violation.
	bool virtualFree(
		std::shared_ptr<ScheduleQueue::PCB> pcb, const size_t start) override;

	//may cause page fault, and it's the only place that can cause page fault
	bool accessMemory(
		std::shared_ptr<ScheduleQueue::PCB> pcb, const size_t segmentNumber, const size_t offset);

	struct MemoryStateEntry
	{
		bool free;//available?
		bool isBegin;
		bool isEnd;
	};

	std::array<MemoryStateEntry, kernel::kMemoryBlocks> memState;

private:
	mutable std::mutex _mutex; // protect all data below

};

