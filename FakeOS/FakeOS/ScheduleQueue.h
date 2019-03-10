#pragma once

#include "Kernel.h"
#include <cstdint>
#include <array>
#include <mutex>

namespace ScheduleQueue
{
	enum State: uint8_t
	{
		kNew,
		kRunning,
		kWaiting,
		kReady
		//kTerminated
	};
	enum Priority: uint8_t
	{
		kTimeCritical,
		kHighest,
		kAboveNormal,
		kNormal,
		kBelowNormal,
		kLowest,
		kIdle
	};
	struct Statistics
	{
		//to calculate how long the process has lived
		std::chrono::time_point<std::chrono::steady_clock> timeCreated; // sizeof(timeCreated) == 8
		std::chrono::steady_clock::duration usedCPUTime; // sizeof(usedCPUTime) == 8
	};
	enum PageStatus: uint8_t
	{
		kInMemory,
		kOndisk,
		kNotUsed
	};
	using PageNumber = size_t;
	using PageTable = std::array<std::pair<PageNumber, PageStatus>, kernel::kMaxPagesPerProcess>;

	struct PCB
	{
		char name[kernel::kMaxProcessNameLength]; //60
		uint16_t pid; //2
		State state;  //1
		Priority priority;//1
		Statistics statistics; //16
		PageTable pageTable;
		//open-file table
	};


	//Didn't encapsulate a thread-safe Queue, because it's to hard to predicate
	//what operation will be used, given that we have different scheduling 
	//algorithm.
	//Store (smart) pointer of PCB in those queues to avoid unnecessary copies

	extern std::mutex newlyCreatedQueueMutex;
	//data structure for newlyCreatedQueue...

	extern std::mutex readyQueueMutex;
	//data structure for readyQueue...

	extern std::mutex waitingQueueMutex;
	//data structure for waitingQueue...
}

