#pragma once

#include "Kernel.h"
#include <memory>
#include <functional>
#include <mutex>
#include <atomic>
#include <condition_variable>

//this class starts a new thread
class ProcessScheduler
{
public:

	enum Method //to be reconsidered...
	{
		kFirstInFirstServe,
		kRoundRobin,
		kPriority,
		kDyanmic
	};

	ProcessScheduler(Method method = kPriority);
	~ProcessScheduler();


	void start();
	void quit();

	void wakeUp();

private:
	void threadFunc();

	Method _scheduleMethod;

	std::atomic<bool> _waken;
	std::atomic<bool> _quit;
	mutable std::mutex _mutex;
	std::condition_variable _condition;
	kernel::ThreadPtr _thread;
};

