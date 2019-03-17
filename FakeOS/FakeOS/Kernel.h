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
	constexpr size_t kPageSize               = 4 * 1024;    // 4 KiB
	constexpr size_t kMemorySize             = 1024 * 1024; // 1 MiB ?
	constexpr size_t kSwapAreaSize           = 1024 * 1024; //always same as above
	constexpr size_t kAddressSpacePerProcess = 256 * 1024;  // 256 KiB ?

	//total available virtual memory == kPhysicalMemorySize + kSwapAreaSize;
	constexpr size_t kMemoryPages = kMemorySize / kPageSize; // =256
	constexpr size_t kSwapAreaPages = kSwapAreaSize / kPageSize; // =256
	constexpr size_t kMaxPagesPerProcess = kAddressSpacePerProcess / kPageSize; // =64



	//global instances' access point
	extern std::unique_ptr<ProcessScheduler> processScheduler;
	extern std::unique_ptr<MemoryAllocator> memoryAllocator;
	extern std::unique_ptr<CPUCore> cpuCore;
	extern std::unique_ptr<FileSystem> fileSystem;

	using ThreadPtr = std::unique_ptr<std::thread, std::function<void(std::thread*)>>;
	void ThreadDeleter(std::thread* t); //customized deleter 
}
