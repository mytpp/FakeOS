#pragma once

#include "Kernel.h"


namespace ScheduleQueue {
	struct PCB;
}

class MemoryManager
{
public:
	MemoryManager() {};
	virtual ~MemoryManager() {}

	//Returning false indicates out of memory/
	//If it returns true, 'start' is the starting loaction of allocated memory.
	virtual bool virtualAllocate(
		/*IN*/ std::shared_ptr<ScheduleQueue::PCB> pcb,
		/*IN*/ const size_t size,
		/*IN OUT*/ size_t& start) = 0; //start of allocated memory

	//'start' is the starting location of memory to free.
	//Returning false indicates memory violation.
	virtual bool virtualFree(
		std::shared_ptr<ScheduleQueue::PCB> pcb, const size_t start) = 0;

	virtual bool accessMemory(
		std::shared_ptr<ScheduleQueue::PCB> pcb, const size_t location) = 0;

	virtual void printMemoryStatistics() { }

	static void printAllocatedMemoryInProcess(std::shared_ptr<ScheduleQueue::PCB> pcb);
};

