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
		//printf("%d\n", this->getQueueNum());
		//cout << this->getQueueNum() << endl;
		if (this->startPtr == NULL) {
			return NULL;
		}
		if (this->startPtr->_value == NULL) {
			printf("error!\n");
			return NULL;
		}
		return this->startPtr->_value;
	}
	ScheduleQueue::PCBNode* back() {
		return this->endPtr;
	}
	void setEndPtr(ScheduleQueue::PCBNode* ptr) {
		this->endPtr = ptr;
	}
	void set_nullptr() {
		if (this->endPtr == NULL) {
			return;
		}
		if (this->endPtr->followPointer != NULL) {
			this->endPtr->followPointer = NULL;
		}
	}
	ScheduleQueue::PCBNode* frontPtr() {
		return this->startPtr;
	}
	void insert_num() {
		this->num++;
	}
	ScheduleQueue::PCBNode* remove_Node(int i) {
		if (i > this->getQueueNum() || i <= 0) {
			printf("error!\n");
			return NULL;
		}
		else {
			if (i == 1) {
				ScheduleQueue::PCBNode* midPtr = this->startPtr;
				this->startPtr = this->startPtr->followPointer;
				this->num--;
				if (this->num == 0) {
					this->startPtr = NULL;
				}
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
		if (this->startPtr == NULL) {
			//printf("start or error\n");
			this->endPtr = new ScheduleQueue::PCBNode;
			this->endPtr->_value = newProcess;
			this->startPtr = this->endPtr;
			this->num++;
			return;
		}
		//printf("wtf\n");
		this->endPtr->followPointer = new ScheduleQueue::PCBNode;
		this->endPtr->followPointer->_value = newProcess;
		this->endPtr = this->endPtr->followPointer;
		//printf("%d\n", this->endPtr->_value->predictedCount);
		this->num++;
	}
	std::shared_ptr<ScheduleQueue::PCB> popProcess() {

		ScheduleQueue::PCBNode* tmpPtr = this->startPtr;
		if (tmpPtr == NULL) {
			this->startPtr = NULL;
			this->endPtr = NULL;
			return NULL;
		}
		else {
			this->startPtr = this->startPtr->followPointer;
			this->num--;
			if (this->num == 0) {
				this->startPtr = NULL;
				this->endPtr = NULL;
			}
			return tmpPtr->_value;
		}
	}
	std::shared_ptr<ScheduleQueue::PCB> getNode(int seq) {
		if (this->num < seq) {
			return nullptr;
		}
		else {
			//printf("%d", seq);
			ScheduleQueue::PCBNode* midPtr = this->startPtr;
			for (int tmpCount = 1; tmpCount < seq-1; tmpCount++) {
				//printf("%d", tmpCount);
				midPtr = midPtr->followPointer;
			}//若seq=1，那么返回头指针，若是2，返回的是第二个指针
			return midPtr->_value;
		}
	}
	ScheduleQueue::PCBNode* getPCBNode(int seq) {
		if (this->num < seq) {
			return nullptr;
		}
		else {
			ScheduleQueue::PCBNode* midPtr = this->startPtr;
			//printf("UsingSeq:%d\n", seq);
			for (int tmpCount = 0; tmpCount < seq-1; tmpCount++) {
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
		return ReadyQueue.front();
	}
	ProcessScheduler(Method method = kPriority);
	~ProcessScheduler();


	//priority may be specified from command line
	//void LoadProcess(const std::string& path, Priority priority);
	void CreateProcess(std::shared_ptr<ScheduleQueue::PCB> newProcess);
	void start();
	void quit();
	void printProcess();
	void wakeUp();

private:
	PCBQueue ReadyQueue;
	PCBQueue WaitingQueue;
	PCBQueue NewlyCreatedQueue;
	void threadFunc();
	/*******************************************************************************
	函数名称：:ProcessSchedule_Preemptive()
	函数功能: 抢占式调度
	参数：
	*******************************************************************************/
	void ProcessSchedule_Preemptive();
	/*******************************************************************************
	函数名称：:ProcessSchedule_Nonpreem()
	函数功能: 非抢占式调度
	参数：
	*******************************************************************************/
	void ProcessSchedule_Nonpreem();
	/*******************************************************************************
	函数名称：:ProcessResort_FCFS()
	函数功能: FCFS结合优先级
	参数：
	*******************************************************************************/
	void ProcessResort_FCFS();
	/*******************************************************************************
	函数名称：: ProcessResort_TimeSlice()
	函数功能: 时间片结合优先级，相当于优先级队列
	参数：
	*******************************************************************************/
	void ProcessResort_TimeSlice();
	/*******************************************************************************
	函数名称：:ProcessResort_SJF()
	函数功能: 短进程优先
	参数：
	*******************************************************************************/
	void ProcessResort_SJF();
	/*******************************************************************************
	函数名称：:ProcessResort_HRRN()
	函数功能: 高响应比优先
	参数：
	*******************************************************************************/
	void ProcessResort_HRRN();
	Method _scheduleMethod;

	std::atomic<bool> _waken;
	std::atomic<bool> _quit;
	mutable std::mutex _mutex;
	std::condition_variable _condition;
	kernel::ThreadPtr _thread;
};
