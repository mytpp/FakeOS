#pragma once

#include <memory>
#include <chrono>
#include <functional>

namespace std {
	class thread;
}

class ProcessScheduler;
class MemoryAllocator;
class CPUCore;
class FileSystem;

namespace kernel
{
	using namespace std::chrono_literals;
	//some constant
	
	//how often the ProcessScheduler is waken
	constexpr auto kScheduleInterval = 1s;

	//how often the CPU thread is waken
	constexpr auto kCPUCycle = 200ms;

	//memory related
	constexpr size_t kPageSize = 4 * 1024; // 4 KiB
	constexpr size_t kPhysicalMemorySize = 128 * 1024 * 1024; //128 MiB ?
	constexpr size_t kSwapAreaSize = 128 * 1024 * 1024; //always same as above
	//total available virtual memory == kPhysicalMemorySize + kSwapAreaSize;
	constexpr size_t kNumPages = (kPhysicalMemorySize + kSwapAreaSize) / kPageSize;
	constexpr size_t kAddressSpacePerProcess = 16 * 1024 * 1024; //16 MiB ?
	constexpr size_t kMaxPagesPerProcess = kAddressSpacePerProcess / kPageSize;

	//used in PCB
	//filesystem should also check this
	constexpr size_t kMaxPathLength = 252; 

	//global instances' access point
	extern std::unique_ptr<ProcessScheduler> processScheduler;
	extern std::unique_ptr<MemoryAllocator> memoryAllocator;
	extern std::unique_ptr<CPUCore> cpuCore;
	extern std::unique_ptr<FileSystem> fileSystem;

	using ThreadPtr = std::unique_ptr<std::thread, std::function<void(std::thread*)>>;
	void ThreadDeleter(std::thread* t); //customized deleter 
}
