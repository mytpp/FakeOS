
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
////,_frames(kMemoryPages)
////,_nextToAllocate(_frames.begin())
////,_freeMemoryPagesNum(kMemoryPages)
////,_frameLocator()
////, _freeSwapAreaPagesNum(kSwapAreaPages)
////,_freeSwapAreaPages()
//{
//    for (size_t i = 0; i < kMemoryPages; i++)
//        _frames.emplace_back(i, 0, nullptr);
//    for (auto frameIt=_frames.begin(); frameIt!=_frames.end(); frameIt++)
//        _frameLocator[(*frameIt).frameNumber] = frameIt;
//    for (size_t i = 0; i < kSwapAreaPages; i++)
//        _freeSwapAreaPages.insert(i + kMemoryPages);
//}

SegmentedMemoryManager::~SegmentedMemoryManager() = default;


//complexity O(kMaxPagesPerProcess + size / kPageSize)
bool SegmentedMemoryManager::virtualAllocate(
                                         std::shared_ptr<PCB> pcb,
                                         const size_t size,
                                         size_t& start)
{
    //assert(0 < size); // refuse to allocate if size<=0
    if (size <= 0 || kMaxSegmentSize < size)
        return false;
    
    int toAllocate = 0;
    auto& segmentTable = *(pcb->segmentTable);
    size_t spaceNeeded = size;
    size_t segmentNum = start; // findout which segment of the process demand for spaces
    //critical section starts
    scoped_lock lock(_mutex);
    //if (pagesNeeded > _freeMemoryPagesNum + _freeSwapAreaPages.size())
    //    return false;
    
    
    //update number of free pages
    if (pagesNeeded <= _freeMemoryPagesNum)
        _freeMemoryPagesNum -= pagesNeeded;
    else
        _freeMemoryPagesNum = 0;
    
    //try to commit physical memory
    while (0 < spaceNeeded)
    {
        size_t frameNumber = _nextToAllocate->frameNumber;
        auto pageTableEntry = pageTable[toAllocate];
        
        //update pcb's page table
        segmentTableEntry.segmentNumber = frameNumber;
        pageTableEntry.free = false;
        pageTableEntry.isBegin = false;
        pageTableEntry.isEnd = false;
        pageTableEntry.inMemory = true;
        
        //adjust _frames and _frameLocator
        _nextToAllocate->pageNumber = toAllocate;
        _nextToAllocate->owner = pcb;
        _frameLocator[frameNumber] = _nextToAllocate;
        _nextToAllocate++;
        
        toAllocate++;
        pagesNeeded--;
    }
    
    assert(pagesNeeded <= _freeSwapAreaPages.size());
    while (0 < pagesNeeded)
    {
        auto pageTableEntry = pageTable[toAllocate];
        
        //update pcb's page table
        pageTableEntry.pageNumber = *(_freeSwapAreaPages.begin());
        pageTableEntry.free = false;
        pageTableEntry.isBegin = false;
        pageTableEntry.isEnd = false;
        pageTableEntry.inMemory = false;
        
        _freeSwapAreaPages.erase(_freeSwapAreaPages.begin());
        toAllocate++;
        pagesNeeded--;
    }
    pageTable[startPage].isBegin = true;
    pageTable[startPage + pagesNeeded - 1].isEnd = true;
    
    return true;
}


//complexity O(endPage - startPage)
bool SegmentedMemoryManager::virtualFree(
                                     std::shared_ptr<PCB> pcb, const size_t startPage)
{
    auto& pageTable = *(pcb->pageTable);
    if (!pageTable[startPage].isBegin)
        return false;
    
    size_t i = startPage - 1;
    
    //critical section starts
    scoped_lock lock(_mutex);
    do
    {
        i++;
        pageTable[i].free = true;
        if (pageTable[i].inMemory)
        {
            auto frame = _frameLocator[pageTable[i].pageNumber];
            bool adjust = false;
            //execution order is important
            if (_nextToAllocate == _frames.end())
                adjust = true;
            _frames.emplace_back(frame->frameNumber, 0, nullptr);
            if (adjust)
                _nextToAllocate = --_frames.end();
            
            _frameLocator.erase(frame->frameNumber);
            _frames.erase(frame);
            _freeMemoryPagesNum++;
        }
        else //pageTable[i] in swap area
        {
            _freeSwapAreaPages.insert(pageTable[i].pageNumber);
        }
    } while (!pageTable[i].isEnd);
    
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
