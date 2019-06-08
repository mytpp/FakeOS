#include "ProcessScheduler.h"
#include "ScheduleQueue.h"
#include <cassert>
#include <iostream>

using namespace std;




ProcessScheduler::ProcessScheduler(Method method)
	:_scheduleMethod(method)
	,_waken(false)
	,_quit(false)
	,_thread(nullptr, kernel::ThreadDeleter)
{
}
void ProcessScheduler::ProcessSchedule_Preemptive() {
	if (this->ReadyQueue.front() == NULL) {
		return;
	}
	if (this->ReadyQueue.front()->state == ScheduleQueue::kRunning) {
		this->ReadyQueue.front()->state = ScheduleQueue::kReady;
	}
	if (this->ReadyQueue.front()->state == ScheduleQueue::kTerminated) {
		//giving a log for end of the Process?
		for (int newlyCreatedCount = 0; newlyCreatedCount < NewlyCreatedQueue.getQueueNum(); newlyCreatedCount++) {
			//std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
			//std::unique_lock<std::mutex> lck_newly(ScheduleQueue::newlyCreatedQueueMutex);
			this->ReadyQueue.addProcess(this->NewlyCreatedQueue.remove_Node(newlyCreatedCount)->_value);
			this->ReadyQueue.getNode(ReadyQueue.getQueueNum())->state = ScheduleQueue::kReady;
			//lck_newly.unlock();
			//lck.unlock();
		}
		//std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
		this->ReadyQueue.popProcess();
		//lck.unlock();
		ProcessResort_FCFS();
	}
	else if (this->ReadyQueue.front()->state == ScheduleQueue::kWaiting) {
		//std::unique_lock<std::mutex> lck_newly(ScheduleQueue::newlyCreatedQueueMutex);
		for (int newlyCreatedCount = 0; newlyCreatedCount < NewlyCreatedQueue.getQueueNum(); newlyCreatedCount++) {
			this->ReadyQueue.addProcess(this->NewlyCreatedQueue.remove_Node(newlyCreatedCount)->_value);
			this->ReadyQueue.getNode(ReadyQueue.getQueueNum())->state = ScheduleQueue::kReady;
		}
		//lck_newly.unlock();
		for (int waitCount = 0; waitCount < this->WaitingQueue.getQueueNum(); waitCount++) {
			ScheduleQueue::PCBNode* midPtr = this->WaitingQueue.getPCBNode(waitCount);
			if (midPtr != NULL) {
				if (midPtr->_value->state == ScheduleQueue::kReady) {
					//std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
					//std::unique_lock<std::mutex> lck_waiting(ScheduleQueue::waitingQueueMutex);
					ScheduleQueue::PCBNode* newNode = this->WaitingQueue.remove_Node(waitCount);
					this->ReadyQueue.addProcess(newNode->_value);
					//lck.unlock();
					//lck_waiting.unlock();
				}
			}
		}
		//std::unique_lock<std::mutex> lck_waiting(ScheduleQueue::waitingQueueMutex);
		this->WaitingQueue.addProcess(this->ReadyQueue.popProcess());
		//lck_waiting.unlock();
		ProcessResort_FCFS();
	}
	else if (this->ReadyQueue.front()->state == ScheduleQueue::kReady) {
		//std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
		//std::unique_lock<std::mutex> lck_newly(ScheduleQueue::newlyCreatedQueueMutex);
		for (int newlyCreatedCount = 0; newlyCreatedCount < NewlyCreatedQueue.getQueueNum(); newlyCreatedCount++) {
			this->ReadyQueue.addProcess(this->NewlyCreatedQueue.remove_Node(newlyCreatedCount)->_value);
			this->ReadyQueue.getNode(ReadyQueue.getQueueNum())->state = ScheduleQueue::kReady;
		}
		this->ReadyQueue.addProcess(this->ReadyQueue.popProcess());
		//lck_newly.unlock();
		//lck.unlock();
		//sort the PCB by ComingTime
		ProcessResort_FCFS();
	}
}

void ProcessScheduler::ProcessSchedule_Nonpreem() {
	if (this->ReadyQueue.front() == NULL) {
		return;
	}
	if (this->ReadyQueue.front()->state == ScheduleQueue::kRunning) {
		std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
		std::unique_lock<std::mutex> lck_newly(ScheduleQueue::newlyCreatedQueueMutex);
		for (int newlyCreatedCount = 0; newlyCreatedCount < NewlyCreatedQueue.getQueueNum(); newlyCreatedCount++) {
			this->ReadyQueue.addProcess(this->NewlyCreatedQueue.remove_Node(newlyCreatedCount)->_value);
			this->ReadyQueue.getNode(ReadyQueue.getQueueNum())->state = ScheduleQueue::kReady;
		}
		this->ReadyQueue.addProcess(this->ReadyQueue.popProcess());
		lck_newly.unlock();
		lck.unlock();
	}
	if (this->ReadyQueue.front()->state == ScheduleQueue::kTerminated) {
		//giving a log for end of the Process?
		for (int newlyCreatedCount = 0; newlyCreatedCount < NewlyCreatedQueue.getQueueNum(); newlyCreatedCount++) {
			std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
			std::unique_lock<std::mutex> lck_newly(ScheduleQueue::newlyCreatedQueueMutex);
			this->ReadyQueue.addProcess(this->NewlyCreatedQueue.remove_Node(newlyCreatedCount)->_value);
			this->ReadyQueue.getNode(ReadyQueue.getQueueNum())->state = ScheduleQueue::kReady;
			lck_newly.unlock();
			lck.unlock();
		}
		std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
		this->ReadyQueue.popProcess();
		lck.unlock();
		ProcessResort_FCFS();
	}
	else if (this->ReadyQueue.front()->state == ScheduleQueue::kWaiting) {
		std::unique_lock<std::mutex> lck_newly(ScheduleQueue::newlyCreatedQueueMutex);
		for (int newlyCreatedCount = 0; newlyCreatedCount < NewlyCreatedQueue.getQueueNum(); newlyCreatedCount++) {
			this->ReadyQueue.addProcess(this->NewlyCreatedQueue.remove_Node(newlyCreatedCount)->_value);
			this->ReadyQueue.getNode(ReadyQueue.getQueueNum())->state = ScheduleQueue::kReady;
		}
		lck_newly.unlock();
		for (int waitCount = 0; waitCount < this->WaitingQueue.getQueueNum(); waitCount++) {
			ScheduleQueue::PCBNode* midPtr = this->WaitingQueue.getPCBNode(waitCount);
			if (midPtr != NULL) {
				if (midPtr->_value->state == ScheduleQueue::kReady) {
					std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
					std::unique_lock<std::mutex> lck_waiting(ScheduleQueue::waitingQueueMutex);
					ScheduleQueue::PCBNode* newNode = this->WaitingQueue.remove_Node(waitCount);
					this->ReadyQueue.addProcess(newNode->_value);
					lck.unlock();
					lck_waiting.unlock();
				}
			}
		}
		std::unique_lock<std::mutex> lck_waiting(ScheduleQueue::waitingQueueMutex);
		this->WaitingQueue.addProcess(this->ReadyQueue.popProcess());
		lck_waiting.unlock();
		ProcessResort_FCFS();
	}
	else if (this->ReadyQueue.front()->state == ScheduleQueue::kReady) {
		std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
		std::unique_lock<std::mutex> lck_newly(ScheduleQueue::newlyCreatedQueueMutex);
		for (int newlyCreatedCount = 0; newlyCreatedCount < NewlyCreatedQueue.getQueueNum(); newlyCreatedCount++) {
			this->ReadyQueue.addProcess(this->NewlyCreatedQueue.remove_Node(newlyCreatedCount)->_value);
			this->ReadyQueue.getNode(ReadyQueue.getQueueNum())->state = ScheduleQueue::kReady;
		}
		this->ReadyQueue.addProcess(this->ReadyQueue.popProcess());
		lck_newly.unlock();
		lck.unlock();
		//sort the PCB by ComingTime
		ProcessResort_FCFS();
	}
}

void ProcessScheduler::ProcessResort_FCFS() {
	//First Based On Priority
	//bubble sort
	shared_ptr<ScheduleQueue::PCB> mid_end_ptr;
	//std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
	int bubbleTmp = this->ReadyQueue.getQueueNum();
	for (int bubbleTmp = this->ReadyQueue.getQueueNum(); bubbleTmp > 0; bubbleTmp--) {
		for (int tmp_counter = 0; tmp_counter < bubbleTmp - 1; tmp_counter++) {
			if (this->ReadyQueue.getNode(tmp_counter)->priority < this->ReadyQueue.getNode(tmp_counter + 1)->priority) {
				ScheduleQueue::PCBNode* midNode = this->ReadyQueue.getPCBNode(tmp_counter);
				if (tmp_counter == 0) {
					this->ReadyQueue.popProcess();
					midNode->followPointer = this->ReadyQueue.frontPtr()->followPointer;
					this->ReadyQueue.frontPtr()->followPointer = midNode;
					 
				}
				else {
					this->ReadyQueue.getPCBNode(tmp_counter - 1)->followPointer = midNode->followPointer;
					midNode->followPointer = this->ReadyQueue.getPCBNode(tmp_counter + 1);
					this->ReadyQueue.getPCBNode(tmp_counter + 1)->followPointer = midNode;
					 
				}
			}
			else if (this->ReadyQueue.getNode(tmp_counter)->statistics.timeCreated > this->ReadyQueue.getNode(tmp_counter + 1)->statistics.timeCreated) {
				ScheduleQueue::PCBNode* midNode = this->ReadyQueue.getPCBNode(tmp_counter);
				if (tmp_counter == 0) {
					this->ReadyQueue.popProcess();
					midNode->followPointer = this->ReadyQueue.frontPtr()->followPointer;
					this->ReadyQueue.frontPtr()->followPointer = midNode;
					 
				}
				else {
					this->ReadyQueue.getPCBNode(tmp_counter - 1)->followPointer = midNode->followPointer;
					midNode->followPointer = this->ReadyQueue.getPCBNode(tmp_counter + 1);
					this->ReadyQueue.getPCBNode(tmp_counter + 1)->followPointer = midNode;
					 
				}
			}
		}
	}
	//lck.unlock();
}

void ProcessScheduler::ProcessResort_TimeSlice() {
	shared_ptr<ScheduleQueue::PCB> mid_end_ptr;
	std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
	int bubbleTmp = this->ReadyQueue.getQueueNum();
	for (int bubbleTmp = this->ReadyQueue.getQueueNum(); bubbleTmp > 0; bubbleTmp--) {
		for (int tmp_counter = 0; tmp_counter < bubbleTmp - 1; tmp_counter++) {
			if (this->ReadyQueue.getNode(tmp_counter)->priority < this->ReadyQueue.getNode(tmp_counter + 1)->priority) {
				ScheduleQueue::PCBNode* midNode = this->ReadyQueue.getPCBNode(tmp_counter);
				if (tmp_counter == 0) {
					this->ReadyQueue.popProcess();
					midNode->followPointer = this->ReadyQueue.frontPtr()->followPointer;
					this->ReadyQueue.frontPtr()->followPointer = midNode;
					 
				}
				else {
					this->ReadyQueue.getPCBNode(tmp_counter - 1)->followPointer = midNode->followPointer;
					midNode->followPointer = this->ReadyQueue.getPCBNode(tmp_counter + 1);
					this->ReadyQueue.getPCBNode(tmp_counter + 1)->followPointer = midNode;
					 
				}
			}
		}
	}
	lck.unlock();
}


void ProcessScheduler::CreateProcess(shared_ptr<ScheduleQueue::PCB> newProcess) {
	this->NewlyCreatedQueue.addProcess(newProcess);
}

void ProcessScheduler::printProcess() {
	if (this->ReadyQueue.getQueueNum() == 0 && this->NewlyCreatedQueue.getQueueNum() == 0 && this->WaitingQueue.getQueueNum() == 0) {
		cout<< "empty" <<endl;
		return;
	}
	else {
		cout << "ReadyQueue:" << endl;
		for (int i = 0; i < this->ReadyQueue.getQueueNum(); i++) {
			if (this->ReadyQueue.getPCBNode(i)->_value->state == ScheduleQueue::kRunning) {
				cout << "RunningPCB" << i << endl;
				cout << "pid:" << this->ReadyQueue.getPCBNode(i)->_value->pid << endl;
				cout << "restCode:" << this->ReadyQueue.getPCBNode(i)->_value->restCode << endl;
				cout << "priority:" << this->ReadyQueue.getPCBNode(i)->_value->priority << endl;
				cout << "predictedCount:" << this->ReadyQueue.getPCBNode(i)->_value->predictedCount << endl;
				cout << "programCount:" << this->ReadyQueue.getPCBNode(i)->_value->programCounter << endl;
				cout << endl << endl;
			}
			cout << "PCB" << i << endl;
			cout << "pid:" << this->ReadyQueue.getPCBNode(i)->_value->pid << endl;
			cout << "restCode:" << this->ReadyQueue.getPCBNode(i)->_value->restCode << endl;
			cout << "priority:" << this->ReadyQueue.getPCBNode(i)->_value->priority << endl;
			cout << "predictedCount:" << this->ReadyQueue.getPCBNode(i)->_value->predictedCount << endl;
			cout << "programCount:" << this->ReadyQueue.getPCBNode(i)->_value->programCounter << endl;
		}
		cout << "NewlyCreatedQueue:" << endl;
		for (int i = 0; i < this->NewlyCreatedQueue.getQueueNum(); i++) {
			cout << "PCB" << i << endl;
			cout << "pid:" << this->NewlyCreatedQueue.getPCBNode(i)->_value->pid << endl;
			cout << "restCode:" << this->NewlyCreatedQueue.getPCBNode(i)->_value->restCode << endl;
			cout << "priority:" << this->NewlyCreatedQueue.getPCBNode(i)->_value->priority << endl;
			cout << "predictedCount:" << this->NewlyCreatedQueue.getPCBNode(i)->_value->predictedCount << endl;
			cout << "programCount:" << this->NewlyCreatedQueue.getPCBNode(i)->_value->programCounter << endl;
		}
		cout << "WaitingQueue:" << endl;
		for (int i = 0; i < this->WaitingQueue.getQueueNum(); i++) {
			cout << "PCB" << i << endl;
			cout << "pid:" << this->WaitingQueue.getPCBNode(i)->_value->pid << endl;
			cout << "restCode:" << this->WaitingQueue.getPCBNode(i)->_value->restCode << endl;
			cout << "priority:" << this->WaitingQueue.getPCBNode(i)->_value->priority << endl;
			cout << "predictedCount:" << this->WaitingQueue.getPCBNode(i)->_value->predictedCount << endl;
			cout << "programCount:" << this->WaitingQueue.getPCBNode(i)->_value->programCounter << endl;
		}
		return;
	}
}

ProcessScheduler::~ProcessScheduler()
{
}

void ProcessScheduler::ProcessResort_HRRN() {
	//First Based On Priority
	//bubble sort
	shared_ptr<ScheduleQueue::PCB> mid_end_ptr;
	std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
	int bubbleTmp = this->ReadyQueue.getQueueNum();
	for (int bubbleTmp = this->ReadyQueue.getQueueNum(); bubbleTmp > 0; bubbleTmp--) {
		for (int tmp_counter = 0; tmp_counter < bubbleTmp - 1; tmp_counter++) {
			if (this->ReadyQueue.getNode(tmp_counter)->priority < this->ReadyQueue.getNode(tmp_counter + 1)->priority) {
				ScheduleQueue::PCBNode* midNode = this->ReadyQueue.getPCBNode(tmp_counter);
				if (tmp_counter == 0) {
					this->ReadyQueue.popProcess();
					midNode->followPointer = this->ReadyQueue.frontPtr()->followPointer;
					this->ReadyQueue.frontPtr()->followPointer = midNode;
					 
				}
				else {
					this->ReadyQueue.getPCBNode(tmp_counter - 1)->followPointer = midNode->followPointer;
					midNode->followPointer = this->ReadyQueue.getPCBNode(tmp_counter + 1);
					this->ReadyQueue.getPCBNode(tmp_counter + 1)->followPointer = midNode;
					 
				}
			}
			else if (this->ReadyQueue.getNode(tmp_counter)->statistics.timeCreated - this->ReadyQueue.getNode(tmp_counter + 1)->statistics.timeCreated < 
				this->ReadyQueue.getNode(tmp_counter + 1)->predictedCount - this->ReadyQueue.getNode(tmp_counter)->predictedCount) {
				ScheduleQueue::PCBNode* midNode = this->ReadyQueue.getPCBNode(tmp_counter);
				if (tmp_counter == 0) {
					this->ReadyQueue.popProcess();
					midNode->followPointer = this->ReadyQueue.frontPtr()->followPointer;
					this->ReadyQueue.frontPtr()->followPointer = midNode;
					 
				}
				else {
					this->ReadyQueue.getPCBNode(tmp_counter - 1)->followPointer = midNode->followPointer;
					midNode->followPointer = this->ReadyQueue.getPCBNode(tmp_counter + 1);
					this->ReadyQueue.getPCBNode(tmp_counter + 1)->followPointer = midNode;
					 
				}
			}
		}
	}
	lck.unlock();
}

void ProcessScheduler::ProcessResort_SJF() {
	//First Based On Priority
	//bubble sort
	shared_ptr<ScheduleQueue::PCB> mid_end_ptr;
	std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
	int bubbleTmp = this->ReadyQueue.getQueueNum();
	for (int bubbleTmp = this->ReadyQueue.getQueueNum(); bubbleTmp > 0; bubbleTmp--) {
		for (int tmp_counter = 0; tmp_counter < bubbleTmp - 1; tmp_counter++) {
			if (this->ReadyQueue.getNode(tmp_counter)->priority < this->ReadyQueue.getNode(tmp_counter + 1)->priority) {
				ScheduleQueue::PCBNode* midNode = this->ReadyQueue.getPCBNode(tmp_counter);
				if (tmp_counter == 0) {
					this->ReadyQueue.popProcess();
					midNode->followPointer = this->ReadyQueue.frontPtr()->followPointer;
					this->ReadyQueue.frontPtr()->followPointer = midNode;
					 
				}
				else {
					this->ReadyQueue.getPCBNode(tmp_counter - 1)->followPointer = midNode->followPointer;
					midNode->followPointer = this->ReadyQueue.getPCBNode(tmp_counter + 1);
					this->ReadyQueue.getPCBNode(tmp_counter + 1)->followPointer = midNode;
					 
				}
			}
			else if (this->ReadyQueue.getNode(tmp_counter)->predictedCount < this->ReadyQueue.getNode(tmp_counter + 1)->predictedCount) {
				ScheduleQueue::PCBNode* midNode = this->ReadyQueue.getPCBNode(tmp_counter);
				if (tmp_counter == 0) {
					this->ReadyQueue.popProcess();
					midNode->followPointer = this->ReadyQueue.frontPtr()->followPointer;
					this->ReadyQueue.frontPtr()->followPointer = midNode;
					 
				}
				else {
					this->ReadyQueue.getPCBNode(tmp_counter - 1)->followPointer = midNode->followPointer;
					midNode->followPointer = this->ReadyQueue.getPCBNode(tmp_counter + 1);
					this->ReadyQueue.getPCBNode(tmp_counter + 1)->followPointer = midNode;
					 
				}
			}
		}
	}
	lck.unlock();
}

void ProcessScheduler::start()
{
	assert(!_thread);
	_quit = false;
	_thread.reset(new thread(
		bind(&ProcessScheduler::threadFunc, this)
	));
}

void ProcessScheduler::quit()
{
	_quit = true;
}

void ProcessScheduler::wakeUp()
{
	if (!_waken) 
	{
		_waken = true;
		_condition.notify_one();
	}
}

void ProcessScheduler::threadFunc()
{
	while (!_quit)
	{
		unique_lock conditionLock(_mutex);
		_condition.wait(conditionLock, [this] { return _waken.load(); });
		
		scoped_lock queueLock(
			ScheduleQueue::newlyCreatedQueueMutex, 
			ScheduleQueue::readyQueueMutex,
			ScheduleQueue::waitingQueueMutex
		);
		//code here
		//algorithm for scheduling
		/*Have no idea know what should be done here.*/
		this->ProcessSchedule_Preemptive();
		//switch...case...
		

		_waken = false;
	}
}
