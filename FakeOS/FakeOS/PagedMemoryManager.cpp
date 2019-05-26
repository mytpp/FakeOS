#include "PagedMemoryManager.h"
#include "ScheduleQueue.h"
#include <cassert>
#include <numeric> 
#include <iterator>

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
	for (auto frameIt=_frames.begin(); frameIt!=_frames.end(); frameIt++)
		_frameLocator[(*frameIt).frameNumber] = frameIt;
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
	//assert(0 < size); // refuse to allocate if size<=0
	if (size <= 0 || kMaxPagesPerProcess < size)
		return false;

	int toAllocate = 0;
	auto& pageTable = *(pcb->pageTable);
	size_t pagesNeeded = size % kPageSize == 0 ?
			size / kPageSize : size / kPageSize + 1;

	//critical section starts
	scoped_lock lock(_mutex);
	if (pagesNeeded > _freeMemoryPagesNum + _freeSwapAreaPages.size())
		return false;

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
		return false;

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
		auto pageTableEntry = pageTable[toAllocate];

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
bool PagedMemoryManager::accessMemory(
	std::shared_ptr<ScheduleQueue::PCB> pcb, const size_t pageNumber)
{
	auto& pageTableEntry = (*(pcb->pageTable))[pageNumber];
	if (pageTableEntry.free)
		return false;

	scoped_lock lock(_mutex);
	if (!pageTableEntry.inMemory)
	{//not in memory
		_freeSwapAreaPages.insert(pageTableEntry.pageNumber);
		if (_nextToAllocate != _frames.end())
		{//memory is not full
			_nextToAllocate->owner = pcb;
			pageTableEntry.pageNumber = _nextToAllocate->frameNumber;
			pageTableEntry.inMemory = true;
		}
		else // LRU replacement
		{
			//swap out the least recently used frame
			auto& lruPage = _frames.front();
			auto& anotherPageTable = *(lruPage.owner->pageTable);
			auto& anotherPageTableEntry = anotherPageTable[lruPage.pageNumber];
			assert(anotherPageTableEntry.free == false);
			anotherPageTableEntry.inMemory = false;
			assert(!_freeSwapAreaPages.empty());
			anotherPageTableEntry.pageNumber = *_freeSwapAreaPages.begin();
			_freeSwapAreaPages.erase(_freeSwapAreaPages.begin());

			//swap in the new page
			_frames.emplace_back(lruPage.frameNumber, pageNumber, pcb);
			_frames.pop_front();
		}
	}
	else
	{
		//move the accessed frame to the end of _frames.
		auto frame = _frameLocator[pageTableEntry.pageNumber];
		_frames.splice(_frames.end(), _frames, frame);
	}

	return true;
}
