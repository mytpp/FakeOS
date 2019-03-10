#include "ProcessScheduler.h"
#include <cassert>

using namespace std;




ProcessScheduler::ProcessScheduler(Method method)
	:_waken(false)
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
		unique_lock<std::mutex> lock(_mutex);
		_condition.wait(lock, [this] { return _waken.load(); });
		
		//code here
		//algorithm for scheduling 
		//switch...case...
		

		_waken = false;
	}
}
