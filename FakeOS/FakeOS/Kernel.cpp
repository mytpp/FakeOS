#include "Kernel.h"
#include "ProcessScheduler.h"
#include "MemoryManager.h"
#include "CPUCore.h"
#include "FileSystem.h"
#include <thread>

namespace kernel
{
	using namespace std;

	unique_ptr<ProcessScheduler> processScheduler;
	unique_ptr<MemoryManager> memoryManager;
	unique_ptr<CPUCore> cpuCore;
	unique_ptr<FileSystem> fileSystem;

	void ThreadDeleter(thread* t)
	{
		t->join();
		delete t;
	}

}