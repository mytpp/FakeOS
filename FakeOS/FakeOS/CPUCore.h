#pragma once

#include "ScheduleQueue.h"
#include <vector>
#include <atomic>
#include <future>
#include "Kernel.h"


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
	bool ParseAndDoDirective(const std::string& directive, uint16_t file_ptr);

	struct IOChannel
	{
		std::shared_ptr<ScheduleQueue::PCB> pcb;
		std::future<bool> event;
	};

	std::vector<IOChannel> _ioChannel;
	std::atomic<bool> _quit;
	kernel::ThreadPtr _thread;
	int64_t _timeElapsed;
};

