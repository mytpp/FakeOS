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

	struct PageTableEntry
	{
		size_t pageNumber;
		bool free;     //if is's true, the three fields below are invalid
		bool inMemory; //false means 'on disk'
		bool begin;    //is beginning of a block of virtual memory
		bool end;      //is end of a block of virtual memory, used by MemoryManager::virtualFree
	};
	using PageTable = std::array<PageTableEntry, kernel::kMaxPagesPerProcess>;


	//--------------------------------------------------------------------------
	//								IMPORTANT
	// Only CPUCore and MemoryManager can modify a process's PCB, ProcessScheduler 
	// is not allowed.
	// Only ProcessScheduler can modify those *Queue, CPUCore and MemoryManager
	// is not allowed.
	//--------------------------------------------------------------------------

	struct PCB  //non-copyable
	{
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
		std::map<std::string, size_t> allocatedMemory; //12
		std::unique_ptr<PageTable> pageTable; //4
		//open-file table
	};
	//int a = sizeof(PCB); //DEBUG
	
	
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

