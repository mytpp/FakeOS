#include "MemoryManager.h"
#include "ScheduleQueue.h"
#include <iostream>

using namespace std;

void MemoryManager::printPagedMemoryInProcess(std::shared_ptr<ScheduleQueue::PCB> pcb)
{
	auto& pageTable = *(pcb->pageTable);
	cout << "Process's (pid=" << pcb->pid << ") allocated memory:" << endl;
	bool noFreePage = true;
	for (size_t i = 0; i < pageTable.size(); i++)
	{
		if (!pageTable[i].free)
		{
			noFreePage = false;
			cout << "pages[" << i << "]:  frame#: " << pageTable[i].pageNumber
				<< boolalpha << " In memory:" << pageTable[i].inMemory;
			if (pageTable[i].isBegin)
				cout << " (Start page)";
			if (pageTable[i].isEnd)
				cout << " (End page) ";
			cout << endl;
		}
	}
	if (noFreePage)
		cout << "None" << endl;
}

void MemoryManager::printSegmentedMemoryInProcess(std::shared_ptr<ScheduleQueue::PCB> pcb)
{
	auto& segmentTable = *(pcb->segmentTable);
	cout << "Process's (pid=" << pcb->pid << ") allocated memory:" << endl;
	bool noFreePage = true;
	for (size_t i = 0; i < segmentTable.size(); i++)
	{
		if (segmentTable[i].segmentSize != 0)
			cout << "segment[" << i << "]:  base#: " << segmentTable[i].base << "  segmentsize" << segmentTable[i].segmentSize << endl;
	}
		cout << "None" << endl;
}
