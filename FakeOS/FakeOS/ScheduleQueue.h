#pragma once

#include "Kernel.h"
#include <cstdint>
#include <string>
#include <array>
#include <mutex>

namespace ScheduleQueue
{
	enum State: uint8_t
	{
		kNew,
		kRunning,
		kWaiting,
		kReady,
		kTerminated
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


	//--------------------------------------------------------------------------
	//								IMPORTANT
	// Only CPUCore can modify a process's PCB, ProcessScheduler is not allowed.
	// Only ProcessScheduler can modify those *Queue, CPUCore is not allowed.
	//--------------------------------------------------------------------------

	struct PCB
	{	
		//prohibit copying
		//PCB should can only be accessed by pointer
		PCB(const PCB&) = delete; 

		//process's name can be extracted from path
		std::string path; //28
		//use std::string because we don't know the exact size at compile time
		uint16_t pid; //2
		State state;  //1
		Priority priority; //1
		std::string textSegment; //28
		//Not real program counter like what were taoght in OS course,
		//this means how many CPU cycle is left before executing next 
		//directive. Initial value is 0.
		size_t programCounter; //4
		Statistics statistics; //16
		PageTable pageTable;
		//open-file table
	};
	//int a = sizeof(std::string); //DEBUG
	
	
	//priority may be specified from command line
	void LoadProcess(const std::string& path, Priority priority);


	//Didn't encapsulate a thread-safe Queue, because it's to hard to predicate
	//what operation will be used, given that we have different scheduling 
	//algorithm.
	//Store (smart) pointer of PCB in those queues to avoid unnecessary copies

	extern std::mutex newlyCreatedQueueMutex;
	//data structure for newlyCreatedQueue...

	extern std::mutex readyQueueMutex;
	//data structure for readyQueue...
	//the head of readyQueue's state is kRunning

	extern std::mutex waitingQueueMutex;
	//data structure for waitingQueue...
}

