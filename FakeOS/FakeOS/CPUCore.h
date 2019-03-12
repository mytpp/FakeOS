#pragma once

#include "ScheduleQueue.h"
#include <vector>
#include <atomic>
#include <future>


//this class starts a new thread
//Simulate the running of processes.
class CPUCore
{
public:

	CPUCore();
	~CPUCore();

	void start();
	void quit();

private:
	void threadFunc();

	struct IOChannel
	{
		std::unique_ptr<ScheduleQueue::PCB> pcb;
		std::future<bool> event;
	};

	std::vector<IOChannel> _ioChannel;
	std::atomic<bool> _quit;
	kernel::ThreadPtr _thread;
};

