#pragma once

#include "Kernel.h"
#include <atomic>


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


	std::atomic<bool> _quit;
	kernel::ThreadPtr _thread;
};

