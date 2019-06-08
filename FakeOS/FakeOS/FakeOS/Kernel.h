#pragma once

#include <memory>
#include <chrono>
#include <functional>

namespace std {
	class thread;
}

class ProcessScheduler;
class MemoryManager;
class CPUCore;
class FileSystem;

namespace kernel
{
	using namespace std::chrono_literals;

	//some constant

	//how often the ProcessScheduler is waken
	constexpr auto kScheduleInterval = 5;

	//how often the CPU thread is waken
	constexpr auto kCPUCycle = 200ms;

	//memory related
	constexpr size_t kPageSize = 16 * 1024;    // 4 KiB
	constexpr size_t kMemorySize = 1024 * 1024; // 1 MiB ?
	constexpr size_t kSwapAreaSize = 1024 * 1024; //always same as above
	constexpr size_t kAddressSpacePerProcess = 256 * 1024;  // 256 KiB ?
	constexpr size_t kMaxSegmentSize = 64 * 1024;//64 KiB
	constexpr size_t kMaxSegmentNum = 16;
	constexpr size_t kUnitMemoryblock = 1024;      //

	//total available virtual memory == kPhysicalMemorySize + kSwapAreaSize;
	constexpr size_t kMemoryPages = kMemorySize / kPageSize; // =64
	constexpr size_t kSwapAreaPages = kSwapAreaSize / kPageSize; // =64
	constexpr size_t kMaxPagesPerProcess = kAddressSpacePerProcess / kPageSize; // =16
	constexpr size_t kMemoryBlocks = kMemorySize / kUnitMemoryblock; //1024


	//global instances' access point
	extern std::unique_ptr<ProcessScheduler> processScheduler;
	extern std::unique_ptr<MemoryManager> memoryManager;
	extern std::unique_ptr<CPUCore> cpuCore;
	extern std::unique_ptr<FileSystem> fileSystem;

	using ThreadPtr = std::unique_ptr<std::thread, std::function<void(std::thread*)>>;
	void ThreadDeleter(std::thread* t); //customized deleter 
}
