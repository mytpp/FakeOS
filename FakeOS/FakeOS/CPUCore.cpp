#include "CPUCore.h"
#include "ProcessScheduler.h"
#include <cassert>

using namespace std;



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
		
		//code here...

		//if (reach kScheduleInterval)
		//wake up ProcessScheduler

	}
}
