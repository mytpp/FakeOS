#include "SegmentedMemoryManager.h"
#include "ScheduleQueue.h"
#include <cassert>
#include <numeric> 
#include <iterator>
#include <iostream>

using namespace std;
using namespace kernel;
using namespace ScheduleQueue;

SegmentedMemoryManager::SegmentedMemoryManager()
	:_mutex()
{
	auto& memoryEntry = memState[0];
	for (size_t i = 0; i < kernel::kMemoryBlocks; i++)
	{
		memState[i].free = true;
	}
}
SegmentedMemoryManager::~SegmentedMemoryManager() = default;


bool SegmentedMemoryManager::virtualAllocate(
	std::shared_ptr<PCB> pcb,
	const size_t size,
	size_t& start)
{
	//assert(0 < size); // refuse to allocate if size<=0
	if (size <= 0 || kMaxSegmentSize < size)
		return false;
	if (start > kMaxSegmentNum) return false;

	int toAllocate = 0;
	auto& segmentTable = *(pcb->segmentTable);
	//auto& memState = *(memoryState);
	size_t spaceNeeded = size;
	size_t segmentNum = start; // findout which segment of the process demand for spaces
	//critical section starts
	scoped_lock lock(_mutex);
	int blocksNeeded = spaceNeeded % kUnitMemoryblock == 0 ?
		spaceNeeded / kUnitMemoryblock : spaceNeeded / kUnitMemoryblock + 1;
	auto& segmentTableEntry = segmentTable[segmentNum];
	int baseBlock = segmentTableEntry.base / kUnitMemoryblock;
	auto& memoryEntry = memState[baseBlock];
	//if the segment has already been allocated
	if (segmentTableEntry.segmentSize != 0)
	{
		while (0 < blocksNeeded)
		{
			if (!memoryEntry.free) return false;//no enough room for new memory command
			blocksNeeded--;
		}
		for (int i = baseBlock; i <= baseBlock + blocksNeeded; i++)
		{

			memState[i].free = false;
			if (i == 1)
				memState[i].isBegin = true;
			else memState[i].isBegin = false;
			if (i == blocksNeeded)
				memState[i].isEnd = true;
			else memState[i].isEnd = false;
		}
		segmentTableEntry.segmentSize += blocksNeeded * kUnitMemoryblock;
		start = segmentTableEntry.segmentSize;
		return true;
	}
	else
	{
		toAllocate = blocksNeeded;
		for (baseBlock = 0; baseBlock < (kMemoryBlocks - blocksNeeded); baseBlock++)
		{
			if (!memState[baseBlock].free) {
				toAllocate = blocksNeeded;
				baseBlock++;
			}
			if (toAllocate == 0)
			{
				segmentTableEntry.base = baseBlock * kUnitMemoryblock;
				segmentTableEntry.segmentSize = spaceNeeded;
				for (int j = baseBlock; j <= baseBlock + blocksNeeded; j++)
				{

					memState[j].free = false;
					if (j == baseBlock)
						memState[j].isBegin = true;
					else memState[j].isBegin = false;
					if (j == baseBlock + blocksNeeded)
						memState[j].isEnd = true;
					else memState[j].isEnd = false;
				}
				start = segmentTableEntry.segmentSize;
				return true;
			}
			toAllocate--;
		}
		return false;
	}
}


//complexity O(endPage - startPage)
bool SegmentedMemoryManager::virtualFree(
	std::shared_ptr<PCB> pcb, const size_t start)
{
	auto& segmentTable = *(pcb->segmentTable);
	//auto& memState = *(memoryState);
	int baseBlock = segmentTable[start].base % kUnitMemoryblock == 0 ?
		segmentTable[start].base / kUnitMemoryblock : segmentTable[start].base / kUnitMemoryblock + 1;
	auto& memoryEntry = memState[baseBlock];
	if (!memoryEntry.isBegin)
		return false;

	size_t i = baseBlock - 1;

	//critical section starts
	scoped_lock lock(_mutex);
	do
	{
		i++;
		memState[i].free = true;
	} while (!memState[i].isEnd);
	segmentTable[start].base = 0;
	segmentTable[start].segmentSize = 0;
	return true;
}


//complexity O(1)
bool SegmentedMemoryManager::accessMemory(
	std::shared_ptr<ScheduleQueue::PCB> pcb, const size_t segmentNumber, const size_t offset)
{
	if (segmentNumber > kernel::kMaxSegmentNum || offset > kMaxSegmentSize) return false;
	auto& segmentTableEntry = (*(pcb->segmentTable))[segmentNumber];
	if (segmentTableEntry.segmentSize == 0 || segmentTableEntry.segmentSize < offset) return false;//if the segmentsize is zero, this segmentnumber in pcb is free; if the offset is bigger than the segmentsize the access is overstep the boundery
	return true;
}

void SegmentedMemoryManager::printMemoryStatistics()
{
	auto& memoryEntry = memState[0];
	for (size_t i = 0; i < kernel::kMemoryBlocks; i++)
	{
		if (memState[i].free) cout << "block:  " << i << "   (free)  " << endl;
		else cout << "block:  " << i << "   (occupied)  " << endl;
	}
	cout << "None" << endl;
}
