#include "MemoryManager.h"
#include "ScheduleQueue.h"
#include <iostream>

using namespace std;

void MemoryManager::printAllocatedMemoryInProcess(std::shared_ptr<ScheduleQueue::PCB> pcb)
{
	auto& pageTable = *(pcb->pageTable);
	cout << "Process's (pid=" << pcb->pid << ") allocated memory:" << endl;
	for (size_t i = 0; i < pageTable.size(); i++)
	{
		if (!pageTable[i].free)
		{
			cout << "pages[" << i << "]:  frame#: " << pageTable[i].pageNumber
				<< boolalpha << " In memory:" << pageTable[i].inMemory << endl;
		}
	}
}