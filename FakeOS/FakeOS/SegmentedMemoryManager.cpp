
#include "SegmentedMemoryManager.h"
#include "ScheduleQueue.h"
#include <cassert>
#include <numeric>
#include <iterator>

using namespace std;
using namespace kernel;
using namespace ScheduleQueue;

SegmentedMemoryManager::SegmentedMemoryManager()
:_mutex()
{
    
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
	auto& memoryState = *(ScheduleQueue::memoryState);
    size_t spaceNeeded = size;
    size_t segmentNum = start; // findout which segment of the process demand for spaces
    //critical section starts
    scoped_lock lock(_mutex);
    size_t blocksNeeded = spaceNeeded % kUnitMemoryblock  == 0 ?
    spaceNeeded /kUnitMemoryblock : spaceNeeded / kUnitMemoryblock + 1;
	auto& segmentTableEntry = segmentTable[segmentNum];
	int baseBlock = segmentTableEntry.base / kUnitMemoryblock;
	auto& memoryEntry = memoryState[baseBlock];
    //if the segment has already been allocated
    if(segmentTableEntry.segmentSize != 0)
    {
        while(0 < blocksNeeded)
        {
            if( !memoryEntry.free ) return false;//no enough room for new memory command
            blocksNeeded-- ;
        }
        for(int i = baseBlock; i <= baseBlock + blocksNeeded ;i++)
        {
            
            memoryState[i].free = false;
            if(i == 1)
                memoryState[i].isBegin = true;
            else memoryState[i].isBegin = false;
            if(i == blocksNeeded)
                memoryState[i].isEnd  = true;
            else memoryState[i].isEnd  = false;
        }
        segmentTableEntry.segmentSize += blocksNeeded * kUnitMemoryblock;
        start = segmentTableEntry.segmentSize;
        return true;
    }
    else
    {
        toAllocate = blocksNeeded;
		for (baseBlock = 0; baseBlock < ( kMemoryBlocks - blocksNeeded); baseBlock++ )
        {
            if(!memoryState[baseBlock].free) {
                toAllocate = blocksNeeded;
                baseBlock++;
            }
            if (toAllocate == 0)
            {
				segmentTableEntry.base = baseBlock * kUnitMemoryblock;
				segmentTableEntry.segmentSize = spaceNeeded;
                for(int j = baseBlock; j <= baseBlock + blocksNeeded ;j++)
                {
                    
                    memoryState[j].free = false;
                    if(j == baseBlock)
                        memoryState[j].isBegin = true;
                    else memoryState[j].isBegin = false;
                    if(j == baseBlock + blocksNeeded)
                        memoryState[j].isEnd  = true;
                    else memoryState[j].isEnd  = false;
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
    size_t baseBlock = segmentTable[start].base % kUnitMemoryblock == 0?
    segmentTable[start].base / kUnitMemoryblock : segmentTable[start].base / kUnitMemoryblock + 1;
    auto& memoryEntry = memoryState[baseBlock];
    if (!memoryEntry.isBegin)
        return false;
    
    size_t i = baseBlock - 1;
    
    //critical section starts
    scoped_lock lock(_mutex);
    do
    {
        i++;
        memoryEntry[i].free = true;
    } while (!memoryEntry[i].isEnd);
    segmentTable[start].base = 0;
    segmentTable[start].segmentSize = 0;
    return true;
}


//complexity O(1)
bool SegmentedMemoryManager::accessMemory(
                                      std::shared_ptr<ScheduleQueue::PCB> pcb, const size_t segmentNumber, const size_t offset)
{
    if(segmentNumber > kernel::kMaxSegmentNum || offset > kMaxSegmentSize) return false;
    auto& segmentTableEntry = (*(pcb->segmentTable))[segmentNumber];
    if(segmentTableEntry.segmentSize==0 || segmentTableEntry.segmentSize < offset) return false;//if the segmentsize is zero, this segmentnumber in pcb is free; if the offset is bigger than the segmentsize the access is overstep the boundery
    return true;
}
