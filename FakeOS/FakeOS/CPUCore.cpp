#include "CPUCore.h"
#include "ProcessScheduler.h"
#include "MemoryAllocator.h"
#include "FileSystem.h"
#include <cassert>

using namespace std;

bool ParseAndDoDirective(const string& directive)
{
	//interact with MemoryAllocator and FileSystem
	//code here...

	return false; //if directive is unrecognizable
}

CPUCore::CPUCore()
	:_quit(false)
	,_thread(nullptr, kernel::ThreadDeleter)
{
}


CPUCore::~CPUCore()
{
}

void CPUCore::start()
{
	assert(!_thread);
	_quit = false;
	_thread.reset(new thread(
		bind(&CPUCore::threadFunc, this)
	));
}

void CPUCore::quit()
{
	_quit = true;
}

void CPUCore::threadFunc()
{
	while (!_quit)
	{
		this_thread::sleep_for(kernel::kCPUCycle);
		
		//if (reach kScheduleInterval)
		//wake up ProcessScheduler
		
		{//critical section
			scoped_lock lock(ScheduleQueue::readyQueueMutex);

			//peek the top element of kernel::readyQueue
			//if programCounter reach 0, call ParseAndDoDirective()

		}

		{//critical section
			scoped_lock lock(ScheduleQueue::waitingQueueMutex);

			//poll _ioChannel to see if there is IO event ready,
			//and change the corresponding PCB's state to ready

		}
		
	}
}
