#include "PagedMemoryManager.h"
#include "ScheduleQueue.h"
#include <cassert>
#include <numeric> 
#include <iterator>
#include <iostream>

using namespace std;
using namespace kernel;
using namespace ScheduleQueue;

PagedMemoryManager::PagedMemoryManager()
	:_mutex()
	//,_frames(kMemoryPages)
	//,_nextToAllocate(_frames.begin())
	,_freeMemoryPagesNum(kMemoryPages)
	//,_frameLocator()
	//, _freeSwapAreaPagesNum(kSwapAreaPages)
	//,_freeSwapAreaPages()
{
	for (size_t i = 0; i < kMemoryPages; i++)
		_frames.emplace_back(i, 0, nullptr);
	_nextToAllocate = _frames.begin();
	//for (auto frameIt=_frames.begin(); frameIt!=_frames.end(); frameIt++)
	//	_frameLocator[(*frameIt).frameNumber] = frameIt;
	for (size_t i = 0; i < kSwapAreaPages; i++)
		_freeSwapAreaPages.insert(i + kMemoryPages);
}

PagedMemoryManager::~PagedMemoryManager() = default;


//complexity O(kMaxPagesPerProcess + size / kPageSize)
bool PagedMemoryManager::virtualAllocate(
	std::shared_ptr<PCB> pcb,
	const size_t size,
	size_t& startPage)
{
	// refuse to allocate if size<=0
	if (size <= 0 || kMaxPagesPerProcess * kPageSize < size)
	{
		cout << "Allocate failed: illegal size." << endl;
		return false;
	}

	int toAllocate = 0;
	auto& pageTable = *(pcb->pageTable);
	size_t pagesNeeded = size % kPageSize == 0 ?
			size / kPageSize : size / kPageSize + 1;

	//critical section starts
	scoped_lock lock(_mutex);
	if (pagesNeeded > _freeMemoryPagesNum + _freeSwapAreaPages.size())
	{
		cout << "Allocate failed: no enough memory." << endl;
		return false;
	}

	//try to reserve address space
	//use first fit
	int maxAvailableSpace = 0;
	int i = 0;
	for (; i < kMaxPagesPerProcess; i++)
	{
		if (pageTable[i].free)
		{
			maxAvailableSpace++;
			if (maxAvailableSpace == pagesNeeded)
				break;
		}
		else
		{
			maxAvailableSpace = 0;
			toAllocate = i + 1;
		}
	}
	if (i == kMaxPagesPerProcess) //address space unavailable!
	{
		cout << "Allocate failed: no enough address space." << endl;
		return false;
	}

	startPage = toAllocate;

	//update number of free pages
	if (pagesNeeded <= _freeMemoryPagesNum)
		_freeMemoryPagesNum -= pagesNeeded;
	else
		_freeMemoryPagesNum = 0;
	
	//try to commit physical memory
	while (0 < pagesNeeded && _nextToAllocate != _frames.end())
	{
		size_t frameNumber = _nextToAllocate->frameNumber;
		auto& pageTableEntry = pageTable[toAllocate];

		//update pcb's page table
		pageTableEntry.pageNumber = frameNumber;
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
		auto& pageTableEntry = pageTable[toAllocate];

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
	//value of maxAvailableSpace == initial value of pagesNeeded
	pageTable[startPage + maxAvailableSpace - 1].isEnd = true;

	return true;
}


//complexity O(endPage - startPage)
bool PagedMemoryManager::virtualFree(
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
			_frames.splice(_nextToAllocate, _frames, frame);
			_nextToAllocate = frame;
			_frameLocator.erase(frame->frameNumber);
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
bool PagedMemoryManager::accessMemory(
	std::shared_ptr<ScheduleQueue::PCB> pcb, const size_t pageNumber)
{
	auto& pageTableEntry = (*(pcb->pageTable))[pageNumber];
	if (pageTableEntry.free)
	{
		cout << "Accessing freed memory." << endl;
		return false;
	}

	//The follow commented code lead to an error? WTF?
	//std::list<PageInfoEntry>::iterator allocatedFrame;
	//allocatedFrame = _nextToAllocate; //=>error?
	scoped_lock lock(_mutex);
	if (!pageTableEntry.inMemory)
	{   //not in memory
		_freeSwapAreaPages.insert(pageTableEntry.pageNumber);
		if (_nextToAllocate != _frames.end())
		{   //memory is not full
			_frameLocator[_nextToAllocate->frameNumber] = _nextToAllocate;
			pageTableEntry.pageNumber = _nextToAllocate->frameNumber;
			_nextToAllocate->pageNumber = pageNumber;
			_nextToAllocate->owner = pcb;
			_nextToAllocate++;
			_freeMemoryPagesNum--;
		}
		else // LRU replacement
		{
			//swap out the least recently used frame
			auto lruPage = _frames.begin();
			auto& anotherPageTable = *(lruPage->owner->pageTable);
			auto& anotherPageTableEntry = anotherPageTable[lruPage->pageNumber];
			assert(anotherPageTableEntry.free == false);
			anotherPageTableEntry.inMemory = false;
			assert(!_freeSwapAreaPages.empty());
			anotherPageTableEntry.pageNumber = *_freeSwapAreaPages.begin();
			_freeSwapAreaPages.erase(_freeSwapAreaPages.begin());

			//swap in the new page
			pageTableEntry.pageNumber = lruPage->frameNumber;
			lruPage->owner = pcb;
			lruPage->pageNumber = pageNumber;
			_frames.splice(_nextToAllocate, _frames, lruPage);
			//no need to reset _frameLocator
		}
		pageTableEntry.inMemory = true;
	}
	else
	{
		//move the accessed frame to the end of _frames.
		auto frame = _frameLocator[pageTableEntry.pageNumber];
		_frames.splice(_nextToAllocate, _frames, frame);
		//no need to reset _frameLocator
	}

	return true;
}

void PagedMemoryManager::printMemoryStatistics()
{
	size_t usedMemoryPagesNum = kMemoryPages - _freeMemoryPagesNum;
	size_t usedSwapAreaPagesNum = kSwapAreaPages - _freeSwapAreaPages.size();
	cout << "Free Memory: " << _freeMemoryPagesNum * kPageSize 
		<<"B (" << _freeMemoryPagesNum<<" pages)  "
		<< "Used Memory: " << usedMemoryPagesNum * kPageSize
		<<"B (" << usedMemoryPagesNum << " pages)" << endl;
	cout << "Free Swap Area Space: " << _freeSwapAreaPages.size() * kPageSize 
		<< "B (" << _freeSwapAreaPages.size() << " pages)  "
		<< "Swap Area Used: " << usedSwapAreaPagesNum * kPageSize
		<< "B (" << usedSwapAreaPagesNum << " pages)  " << endl;
	cout << "Frames of Physical Memory (Sorted by used time, least -> most recent): " << endl;
	auto frame = _frames.begin();
	for (size_t i = 0; i < usedMemoryPagesNum; i++)
	{
		cout << frame->frameNumber << "(" << frame->owner->pid << ")" << "->";
		frame++;
	}
	cout << "None" << endl;
}

