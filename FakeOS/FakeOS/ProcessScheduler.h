#pragma once

#include "Kernel.h"
#include <memory>
#include <functional>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "ScheduleQueue.h"


class PCBQueue {
public:
	int getQueueNum() { return num; }
	PCBQueue() {
		this->startPtr = NULL;
		this->endPtr = NULL;
	}
	~PCBQueue() {
		this->startPtr = NULL;
		this->endPtr = NULL;
	}
	std::shared_ptr<ScheduleQueue::PCB> front() {
		if (this->startPtr == NULL) {
			return NULL;
		}
		return this->startPtr->_value;
	}
	ScheduleQueue::PCBNode* frontPtr() {
		return this->startPtr;
	}
	void insert_num() {
		this->num++;
	}
	ScheduleQueue::PCBNode* remove_Node(int i) {
		if (i > this->getQueueNum() || i < 0) {
			return NULL;
		}
		else {
			if (i == 1) {
				ScheduleQueue::PCBNode* midPtr = this->startPtr;
				this->startPtr = startPtr->followPointer;
				this->num--;
				return midPtr;
			}
			else {
				ScheduleQueue::PCBNode* midPtr = this->getPCBNode(i);
				this->getPCBNode(i - 1)->followPointer = this->getPCBNode(i)->followPointer;
				this->num--;
				return midPtr;
			}
		}
	}
	void addProcess(std::shared_ptr<ScheduleQueue::PCB> newProcess) {
		this->endPtr->followPointer = new ScheduleQueue::PCBNode;
		this->endPtr->followPointer->_value = newProcess;
		this->endPtr = this->endPtr->followPointer;
		this->num++;
	}
	std::shared_ptr<ScheduleQueue::PCB> popProcess() {

		ScheduleQueue::PCBNode* tmpPtr = this->startPtr;
		if (tmpPtr == NULL) {
			return NULL;
		}
		else {
			this->startPtr = this->startPtr->followPointer;
			return tmpPtr->_value;
		}
	}
	std::shared_ptr<ScheduleQueue::PCB> getNode(int seq) {
		if (this->num < seq) {
			return nullptr;
		}
		else {
			ScheduleQueue::PCBNode* midPtr = this->startPtr;
			for (int tmpCount = 0; tmpCount < seq; tmpCount++) {
				midPtr = midPtr->followPointer;
			}
			return midPtr->_value;
		}
	}
	ScheduleQueue::PCBNode* getPCBNode(int seq) {
		if (this->num < seq) {
			return nullptr;
		}
		else {
			ScheduleQueue::PCBNode* midPtr = this->startPtr;
			for (int tmpCount = 0; tmpCount < seq; tmpCount++) {
				midPtr = midPtr->followPointer;
			}
			return midPtr;
		}
	}
private:
	ScheduleQueue::PCBNode* startPtr;
	ScheduleQueue::PCBNode* endPtr;
	int num;
};
//this class starts a new thread
//this class starts a new thread
class ProcessScheduler
{
public:

	enum Method //to be reconsidered...
	{
		kFirstInFirstServe,
		kRoundRobin,
		kPriority,
		kDyanmic
	};
	std::shared_ptr<ScheduleQueue::PCB> getRunningProcess() {
		this->ProcessSchedule();
		return ReadyQueue.front();
	}
	ProcessScheduler(Method method = kPriority);
	~ProcessScheduler();


	//priority may be specified from command line
	//void LoadProcess(const std::string& path, Priority priority);
	void CreateProcess(std::shared_ptr<ScheduleQueue::PCB> newProcess);
	void start();
	void quit();

	void wakeUp();

private:
	PCBQueue ReadyQueue;
	PCBQueue WaitingQueue;
	PCBQueue NewlyCreatedQueue;
	void threadFunc();
	void ProcessSchedule();
	void ProcessResort_FCFS();
	void ProcessResort_TimeSlice();

	Method _scheduleMethod;

	std::atomic<bool> _waken;
	std::atomic<bool> _quit;
	mutable std::mutex _mutex;
	std::condition_variable _condition;
	kernel::ThreadPtr _thread;
};
