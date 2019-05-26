#include "ProcessScheduler.h"
#include "ScheduleQueue.h"
#include <cassert>

using namespace std;




ProcessScheduler::ProcessScheduler(Method method)
	:_scheduleMethod(method)
	,_waken(false)
	,_quit(false)
	,_thread(nullptr, kernel::ThreadDeleter)
{
}


ProcessScheduler::~ProcessScheduler()
{
}

void ProcessScheduler::start()
{
	assert(!_thread);
	_quit = false;
	_thread.reset(new thread(
		bind(&ProcessScheduler::threadFunc, this)
	));
}

void ProcessScheduler::quit()
{
	_quit = true;
}

void ProcessScheduler::wakeUp()
{
	if (!_waken) 
	{
		_waken = true;
		_condition.notify_one();
	}
}

void ProcessScheduler::threadFunc()
{
	while (!_quit)
	{
		unique_lock conditionLock(_mutex);
		_condition.wait(conditionLock, [this] { return _waken.load(); });
		
		scoped_lock queueLock(
			ScheduleQueue::newlyCreatedQueueMutex, 
			ScheduleQueue::readyQueueMutex,
			ScheduleQueue::waitingQueueMutex
		);
		//code here
		//algorithm for scheduling 
		//switch...case...
		

		_waken = false;
	}
}
