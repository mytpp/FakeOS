#include "Kernel.h"
#include "CPUCore.h"
#include "ProcessScheduler.h"
#include "MemoryAllocator.h"
#include "FileSystem.h"
#include <iostream>


using namespace std;

int main() 
{
	cout << "Loading kernel..." << endl;

	kernel::fileSystem = make_unique<FileSystem>();
	kernel::fileSystem->start();
	cout << "File system loaded." << endl;

	kernel::processScheduler = make_unique<ProcessScheduler>();
	kernel::processScheduler->start();
	cout << "Process scheduler loaded." << endl;

	kernel::memoryAllocator = make_unique<MemoryAllocator>();
	cout << "Memory allocator loaded." << endl;

	kernel::cpuCore = make_unique<CPUCore>();
	kernel::cpuCore->start();
	cout << "Process simulator loaded." << endl;


	//After initialization finished, main thread runs as cmd parser.
	while (true)
	{
		//there should be a command prompt like '>' or '$'

		//parse command and execute

	}


}