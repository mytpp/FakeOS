#pragma once

#include "MemoryManager.h"
#include <mutex>
#include <vector>
#include <list>
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
                      std::shared_ptr<ScheduleQueue::PCB> pcb, const size_t segmentNumber, const size_t offset)
    
    
private:
    mutable std::mutex _mutex; // protect all data below
    
//    struct SegmentInfoEntry
//    {
//        PageInfoEntry(size_t segment, size_t base, size_t offset, std::shared_ptr<ScheduleQueue::PCB> pcb)
//        :segmentNumber(segment), baseAdr(base), offsetSize(offset), owner(pcb) { }//each process divided into 5 segments,each segment has it's own Segment base and size
//
//        size_t segmentNumber;
//        size_t baseAdr;  //in user address space
//        size_t offsetSize;
//        std::shared_ptr<ScheduleQueue::PCB> owner;
//    };
    
    //data structure for LRU
    
    //Sorted by recent used time.
    //The front is the least recently used, the back is the most recently used
    //std::list<PageInfoEntry> _frames;
    //std::list<PageInfoEntry>::iterator _nextToAllocate;
    //size_t _freeMemoryPagesNum;
    //map frame number to its location in _frames;
    //std::unordered_map<size_t, std::list<PageInfoEntry>::iterator> _frameLocator;
    
    //size_t _freeSwapAreaPagesNum; //==_freeSwapAreaPages.size() at most time
    //std::unordered_set<size_t> _freeSwapAreaPages;
};


