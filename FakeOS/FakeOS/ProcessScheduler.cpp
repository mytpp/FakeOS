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
	//cout << "11111";
	if (this->ReadyQueue.front() == NULL) {
		//cout << "NULL";
		for (int newlyCreatedCount = 1; newlyCreatedCount <= NewlyCreatedQueue.getQueueNum(); newlyCreatedCount++) {
			//cout << "1111";
			std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
			std::unique_lock<std::mutex> lck_newly(ScheduleQueue::newlyCreatedQueueMutex);
			this->ReadyQueue.addProcess(this->NewlyCreatedQueue.popProcess());
			this->ReadyQueue.getNode(ReadyQueue.getQueueNum())->state = ScheduleQueue::kReady;
			lck_newly.unlock();
			lck.unlock();
		}
		return;
	}
	if (this->ReadyQueue.front()->state == ScheduleQueue::kRunning) {
		cout << "Running";
		this->ReadyQueue.front()->state = ScheduleQueue::kReady;
	}
	if (this->ReadyQueue.front()->state == ScheduleQueue::kTerminated) {
		//giving a log for end of the Process?
		cout << "Terminated";
		this->ReadyQueue.popProcess();
		for (int newlyCreatedCount = 1; newlyCreatedCount <= NewlyCreatedQueue.getQueueNum(); newlyCreatedCount++) {
			// << "1111";
			std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
			std::unique_lock<std::mutex> lck_newly(ScheduleQueue::newlyCreatedQueueMutex);
			this->ReadyQueue.addProcess(this->NewlyCreatedQueue.popProcess());
			this->ReadyQueue.getNode(ReadyQueue.getQueueNum())->state = ScheduleQueue::kReady;
			lck_newly.unlock();
			lck.unlock();
		}
		//cout << "2222";
		std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
		//cout << "33333";
		lck.unlock();
		ProcessResort_FCFS();
	}
	else if (this->ReadyQueue.front()->state == ScheduleQueue::kWaiting) {
		cout << "Waiting";
		std::unique_lock<std::mutex> lck_newly(ScheduleQueue::newlyCreatedQueueMutex);
		for (int newlyCreatedCount = 1; newlyCreatedCount <= NewlyCreatedQueue.getQueueNum(); newlyCreatedCount++) {
			// << "1111";
			this->ReadyQueue.addProcess(this->NewlyCreatedQueue.popProcess());
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
		//cout << "Ready";
		std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
		std::unique_lock<std::mutex> lck_newly(ScheduleQueue::newlyCreatedQueueMutex);
		for (int newlyCreatedCount = 1; newlyCreatedCount <= NewlyCreatedQueue.getQueueNum(); newlyCreatedCount++) {
			//cout << "QueueNum:" << NewlyCreatedQueue.getQueueNum() << endl;
			//cout << "NodeCounter" <<this->NewlyCreatedQueue.popProcess()->programCounter << endl;
			this->ReadyQueue.addProcess(this->NewlyCreatedQueue.popProcess());
			this->ReadyQueue.getNode(ReadyQueue.getQueueNum())->state = ScheduleQueue::kReady;

			/*cout << "ReadyQueue:" << endl;
			printf("%d\n", this->ReadyQueue.getQueueNum());
			for (int i = 0; i < this->ReadyQueue.getQueueNum(); i++) {
				if (this->ReadyQueue.getPCBNode(i)->_value->state == ScheduleQueue::kRunning) {
					cout << "RunningPCB" << i << endl;
					cout << "pid:" << this->ReadyQueue.getPCBNode(i)->_value->pid << endl;
					cout << "restCode:" << this->ReadyQueue.getPCBNode(i)->_value->restCode << endl;
					cout << "priority:" << (int)this->ReadyQueue.getPCBNode(i)->_value->priority << endl;
					cout << "predictedCount:" << this->ReadyQueue.getPCBNode(i)->_value->predictedCount << endl;
					cout << "programCount:" << this->ReadyQueue.getPCBNode(i)->_value->programCounter << endl;
					cout << endl << endl;
				}
				cout << endl;
				cout << "PCB" << i << endl;
				cout << "pid:" << this->ReadyQueue.getPCBNode(i)->_value->pid << endl;
				//cout << "state:" << (int)this->ReadyQueue.getPCBNode(i)->_value->state << endl;
				cout << "restCode:" << this->ReadyQueue.getPCBNode(i)->_value->restCode << endl;
				cout << "priority:" << (int)this->ReadyQueue.getPCBNode(i)->_value->priority << endl;
				cout << "predictedCount:" << this->ReadyQueue.getPCBNode(i)->_value->predictedCount << endl;
				cout << "programCount:" << this->ReadyQueue.getPCBNode(i)->_value->programCounter << endl;
			}*/
		}
		//this->printProcess();
		//this->ReadyQueue.addProcess(this->ReadyQueue.popProcess());
		lck_newly.unlock();
		lck.unlock();
		//sort the PCB by ComingTime
		ProcessResort_FCFS();
	}
}

void ProcessScheduler::ProcessSchedule_Nonpreem() {
	//cout << "11111";
	if (this->ReadyQueue.front() == NULL) {
		//cout << "NULL";
		for (int newlyCreatedCount = 1; newlyCreatedCount <= NewlyCreatedQueue.getQueueNum(); newlyCreatedCount++) {
			//cout << "1111";
			std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
			std::unique_lock<std::mutex> lck_newly(ScheduleQueue::newlyCreatedQueueMutex);
			this->ReadyQueue.addProcess(this->NewlyCreatedQueue.popProcess());
			this->ReadyQueue.getNode(ReadyQueue.getQueueNum())->state = ScheduleQueue::kReady;
			lck_newly.unlock();
			lck.unlock();
		}
		return;
	}
	if (this->ReadyQueue.front()->state == ScheduleQueue::kRunning) {
		cout << "Running";
		this->ReadyQueue.front()->state = ScheduleQueue::kReady;
	}
	if (this->ReadyQueue.front()->state == ScheduleQueue::kTerminated) {
		//giving a log for end of the Process?
		cout << "Terminated";
		this->ReadyQueue.popProcess();
		for (int newlyCreatedCount = 1; newlyCreatedCount <= NewlyCreatedQueue.getQueueNum(); newlyCreatedCount++) {
			// << "1111";
			std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
			std::unique_lock<std::mutex> lck_newly(ScheduleQueue::newlyCreatedQueueMutex);
			this->ReadyQueue.addProcess(this->NewlyCreatedQueue.popProcess());
			this->ReadyQueue.getNode(ReadyQueue.getQueueNum())->state = ScheduleQueue::kReady;
			lck_newly.unlock();
			lck.unlock();
		}
		//cout << "2222";
		std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
		//cout << "33333";
		lck.unlock();
		ProcessResort_FCFS();
	}
	else if (this->ReadyQueue.front()->state == ScheduleQueue::kWaiting) {
		cout << "Waiting";
		std::unique_lock<std::mutex> lck_newly(ScheduleQueue::newlyCreatedQueueMutex);
		for (int newlyCreatedCount = 1; newlyCreatedCount <= NewlyCreatedQueue.getQueueNum(); newlyCreatedCount++) {
			// << "1111";
			this->ReadyQueue.addProcess(this->NewlyCreatedQueue.popProcess());
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
		//cout << "Ready";
		std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
		std::unique_lock<std::mutex> lck_newly(ScheduleQueue::newlyCreatedQueueMutex);
		for (int newlyCreatedCount = 1; newlyCreatedCount <= NewlyCreatedQueue.getQueueNum(); newlyCreatedCount++) {
			//cout << "QueueNum:" << NewlyCreatedQueue.getQueueNum() << endl;
			//cout << "NodeCounter" <<this->NewlyCreatedQueue.popProcess()->programCounter << endl;
			this->ReadyQueue.addProcess(this->NewlyCreatedQueue.popProcess());
			this->ReadyQueue.getNode(ReadyQueue.getQueueNum())->state = ScheduleQueue::kReady;

			/*cout << "ReadyQueue:" << endl;
			printf("%d\n", this->ReadyQueue.getQueueNum());
			for (int i = 0; i < this->ReadyQueue.getQueueNum(); i++) {
				if (this->ReadyQueue.getPCBNode(i)->_value->state == ScheduleQueue::kRunning) {
					cout << "RunningPCB" << i << endl;
					cout << "pid:" << this->ReadyQueue.getPCBNode(i)->_value->pid << endl;
					cout << "restCode:" << this->ReadyQueue.getPCBNode(i)->_value->restCode << endl;
					cout << "priority:" << (int)this->ReadyQueue.getPCBNode(i)->_value->priority << endl;
					cout << "predictedCount:" << this->ReadyQueue.getPCBNode(i)->_value->predictedCount << endl;
					cout << "programCount:" << this->ReadyQueue.getPCBNode(i)->_value->programCounter << endl;
					cout << endl << endl;
				}
				cout << endl;
				cout << "PCB" << i << endl;
				cout << "pid:" << this->ReadyQueue.getPCBNode(i)->_value->pid << endl;
				//cout << "state:" << (int)this->ReadyQueue.getPCBNode(i)->_value->state << endl;
				cout << "restCode:" << this->ReadyQueue.getPCBNode(i)->_value->restCode << endl;
				cout << "priority:" << (int)this->ReadyQueue.getPCBNode(i)->_value->priority << endl;
				cout << "predictedCount:" << this->ReadyQueue.getPCBNode(i)->_value->predictedCount << endl;
				cout << "programCount:" << this->ReadyQueue.getPCBNode(i)->_value->programCounter << endl;
			}*/
		}
		//this->printProcess();
		//this->ReadyQueue.addProcess(this->ReadyQueue.popProcess());
		lck_newly.unlock();
		lck.unlock();
		//sort the PCB by ComingTime
		if (this->ReadyQueue.getQueueNum() > 1) {
			if (this->ReadyQueue.front()->programCounter == 0) {
				ProcessResort_FCFS();
			}
		}
	}
}

void ProcessScheduler::ProcessResort_FCFS() {
	//First Based On Priority
	//bubble sort
	if (this->ReadyQueue.getQueueNum() <= 1) {
		return;
	}
	shared_ptr<ScheduleQueue::PCB> mid_end_ptr;
	std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
	int bubbleTmp = this->ReadyQueue.getQueueNum();
	//cout << "bubbleSignal:" << bubbleTmp << endl;
	for (int bubbleTmp = this->ReadyQueue.getQueueNum(); bubbleTmp > 1; bubbleTmp--) {
		for (int tmp_counter = 1; tmp_counter < bubbleTmp; tmp_counter++) {
			//cout << tmp_counter;
			if (this->ReadyQueue.getNode(tmp_counter)->priority > this->ReadyQueue.getNode(tmp_counter + 1)->priority
				|| (this->ReadyQueue.getNode(tmp_counter)->statistics.timeCreated > this->ReadyQueue.getNode(tmp_counter + 1)->statistics.timeCreated
					&& this->ReadyQueue.getNode(tmp_counter)->priority == this->ReadyQueue.getNode(tmp_counter + 1)->priority)) {
				ScheduleQueue::PCBNode* midNode = this->ReadyQueue.getPCBNode(tmp_counter);
				//cout << "bubbleSignal:" << bubbleTmp << endl;
				if (tmp_counter == 1) {
					//cout << "11111";
					this->ReadyQueue.popProcess();
					midNode->followPointer = this->ReadyQueue.frontPtr()->followPointer;
					if (tmp_counter == 2) {
						midNode->followPointer = NULL;
					}
					this->ReadyQueue.insert_num();
					this->ReadyQueue.frontPtr()->followPointer = midNode;
					/*cout << this->ReadyQueue.getQueueNum() << endl;
					for (int i = 1; i <= this->ReadyQueue.getQueueNum(); i++) {
						cout << this->ReadyQueue.getPCBNode(i)->_value->predictedCount << endl;
					}*/
				}
				else {
					//cout << "error!" << endl;
					this->ReadyQueue.getPCBNode(tmp_counter - 1)->followPointer = midNode->followPointer;
					midNode->followPointer = this->ReadyQueue.getPCBNode(tmp_counter)->followPointer;
					this->ReadyQueue.getPCBNode(tmp_counter)->followPointer = midNode;
					/*cout << this->ReadyQueue.getQueueNum() << endl;
					for (int i = 1; i <= this->ReadyQueue.getQueueNum(); i++) {
						cout << this->ReadyQueue.getPCBNode(i)->_value->predictedCount << endl;
					}*/
					this->ReadyQueue.setEndPtr(this->ReadyQueue.getPCBNode(this->ReadyQueue.getQueueNum()));
				}
			}
		}
	}
	/*cout << this->ReadyQueue.getQueueNum() << endl;
	for (int i = 1; i <= this->ReadyQueue.getQueueNum(); i++) {
		cout << this->ReadyQueue.getPCBNode(i)->_value->predictedCount << endl;
	}*/
	lck.unlock();
}

void ProcessScheduler::ProcessResort_TimeSlice() {
	//First Based On Priority
	//bubble sort
	if (this->ReadyQueue.getQueueNum() <= 1) {
		return;
	}
	shared_ptr<ScheduleQueue::PCB> mid_end_ptr;
	std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
	int bubbleTmp = this->ReadyQueue.getQueueNum();
	//cout << "bubbleSignal:" << bubbleTmp << endl;
	for (int bubbleTmp = this->ReadyQueue.getQueueNum(); bubbleTmp > 1; bubbleTmp--) {
		for (int tmp_counter = 1; tmp_counter < bubbleTmp; tmp_counter++) {
			//cout << tmp_counter;
			ScheduleQueue::PCBNode* midNode = this->ReadyQueue.getPCBNode(tmp_counter);
			if (this->ReadyQueue.getNode(tmp_counter)->priority > this->ReadyQueue.getNode(tmp_counter + 1)->priority || (this->ReadyQueue.getNode(tmp_counter)->programCounter == 0 && this->ReadyQueue.getNode(tmp_counter)->priority == this->ReadyQueue.getNode(tmp_counter + 1)->priority)) {
				//cout << "bubbleSignal:" << bubbleTmp << endl;
				if (tmp_counter == 1) {
					//cout << "11111";
					this->ReadyQueue.popProcess();
					midNode->followPointer = this->ReadyQueue.frontPtr()->followPointer;
					if (tmp_counter == 2) {
						midNode->followPointer = NULL;
					}
					this->ReadyQueue.insert_num();
					this->ReadyQueue.frontPtr()->followPointer = midNode;
					this->ReadyQueue.set_nullptr();
					/*cout << this->ReadyQueue.getQueueNum() << endl;
					for (int i = 1; i <= this->ReadyQueue.getQueueNum(); i++) {
						cout << this->ReadyQueue.getPCBNode(i)->_value->predictedCount << endl;
					}*/
				}
				else {
					//cout << "error!" << endl;
					this->ReadyQueue.getPCBNode(tmp_counter - 1)->followPointer = midNode->followPointer;
					midNode->followPointer = this->ReadyQueue.getPCBNode(tmp_counter)->followPointer;
					this->ReadyQueue.getPCBNode(tmp_counter)->followPointer = midNode;
					/*cout << this->ReadyQueue.getQueueNum() << endl;
					for (int i = 1; i <= this->ReadyQueue.getQueueNum(); i++) {
						cout << this->ReadyQueue.getPCBNode(i)->_value->predictedCount << endl;
					}*/
					this->ReadyQueue.setEndPtr(this->ReadyQueue.getPCBNode(this->ReadyQueue.getQueueNum()));
				}
			}
		}
	}
	/*cout << this->ReadyQueue.getQueueNum() << endl;
	for (int i = 1; i <= this->ReadyQueue.getQueueNum(); i++) {
		cout << this->ReadyQueue.getPCBNode(i)->_value->predictedCount << endl;
	}*/
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
		std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
		std::unique_lock<std::mutex> newly_lck(ScheduleQueue::newlyCreatedQueueMutex);
		std::unique_lock<std::mutex> waiting_lck(ScheduleQueue::waitingQueueMutex);
		cout << "ReadyQueue:";
		printf("%d\n", this->ReadyQueue.getQueueNum());
		for (int i = 1; i <= this->ReadyQueue.getQueueNum(); i++) {
			if (this->ReadyQueue.getPCBNode(i)->_value->state == ScheduleQueue::kRunning) {
				cout << "RunningPCB" << i << endl;
				cout << "pid:" << this->ReadyQueue.getPCBNode(i)->_value->pid << endl;
				cout << "restCode:" << this->ReadyQueue.getPCBNode(i)->_value->restCode << endl;
				cout << "priority:" << (int)this->ReadyQueue.getPCBNode(i)->_value->priority << endl;
				cout << "predictedCount:" << this->ReadyQueue.getPCBNode(i)->_value->predictedCount << endl;
				cout << "programCount:" << this->ReadyQueue.getPCBNode(i)->_value->programCounter << endl;
				cout << endl << endl;
			}
			cout << endl;
			cout << "PCB" << i << endl;
			cout << "pid:" << this->ReadyQueue.getPCBNode(i)->_value->pid << endl;
			//cout << "state:" << (int)this->ReadyQueue.getPCBNode(i)->_value->state << endl;
			cout << "restCode:" << this->ReadyQueue.getPCBNode(i)->_value->restCode << endl;
			cout << "priority:" << (int)this->ReadyQueue.getPCBNode(i)->_value->priority << endl;
			cout << "predictedCount:" << this->ReadyQueue.getPCBNode(i)->_value->predictedCount << endl;
			cout << "programCount:" << this->ReadyQueue.getPCBNode(i)->_value->programCounter << endl;
		}
		cout << endl << endl;
		cout << "NewlyCreatedQueue:" << endl;
		for (int i = 0; i < this->NewlyCreatedQueue.getQueueNum(); i++) {
			cout << endl;
			cout << "PCB" << i << endl;
			cout << "pid:" << this->NewlyCreatedQueue.getPCBNode(i)->_value->pid << endl;
			cout << "restCode:" << this->NewlyCreatedQueue.getPCBNode(i)->_value->restCode << endl;
			cout << "priority:" << (int)this->NewlyCreatedQueue.getPCBNode(i)->_value->priority << endl;
			cout << "predictedCount:" << this->NewlyCreatedQueue.getPCBNode(i)->_value->predictedCount << endl;
			cout << "programCount:" << this->NewlyCreatedQueue.getPCBNode(i)->_value->programCounter << endl;
		}
		cout << endl << endl;
		cout << "WaitingQueue:" << endl;
		for (int i = 0; i < this->WaitingQueue.getQueueNum(); i++) {
			cout << endl;
			cout << "PCB" << i << endl;
			cout << "pid:" << this->WaitingQueue.getPCBNode(i)->_value->pid << endl;
			cout << "restCode:" << this->WaitingQueue.getPCBNode(i)->_value->restCode << endl;
			cout << "priority:" << (int)this->WaitingQueue.getPCBNode(i)->_value->priority << endl;
			cout << "predictedCount:" << this->WaitingQueue.getPCBNode(i)->_value->predictedCount << endl;
			cout << "programCount:" << this->WaitingQueue.getPCBNode(i)->_value->programCounter << endl;
		}
		lck.unlock();
		newly_lck.unlock();
		waiting_lck.unlock();
		return;
	}
}

ProcessScheduler::~ProcessScheduler()
{
}

void ProcessScheduler::ProcessResort_HRRN() {
	if (this->ReadyQueue.getQueueNum() <= 1) {
		return;
	}
	shared_ptr<ScheduleQueue::PCB> mid_end_ptr;
	std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
	int bubbleTmp = this->ReadyQueue.getQueueNum();
	//cout << "bubbleSignal:" << bubbleTmp << endl;
	for (int bubbleTmp = this->ReadyQueue.getQueueNum(); bubbleTmp > 1; bubbleTmp--) {
		for (int tmp_counter = 1; tmp_counter < bubbleTmp; tmp_counter++) {
			//cout << tmp_counter;
			if (this->ReadyQueue.getNode(tmp_counter)->priority > this->ReadyQueue.getNode(tmp_counter + 1)->priority
				|| (this->ReadyQueue.getNode(tmp_counter)->statistics.timeCreated - this->ReadyQueue.getNode(tmp_counter + 1)->statistics.timeCreated <
					this->ReadyQueue.getNode(tmp_counter + 1)->predictedCount - this->ReadyQueue.getNode(tmp_counter)->predictedCount)){
				//cout << "bubbleSignal:" << bubbleTmp << endl;
				ScheduleQueue::PCBNode* midNode = this->ReadyQueue.getPCBNode(tmp_counter);
				if (tmp_counter == 1) {
					//cout << "11111";
					this->ReadyQueue.popProcess();
					midNode->followPointer = this->ReadyQueue.frontPtr()->followPointer;
					if (tmp_counter == 2) {
						midNode->followPointer = NULL;
					}
					this->ReadyQueue.insert_num();
					this->ReadyQueue.frontPtr()->followPointer = midNode;
					/*cout << this->ReadyQueue.getQueueNum() << endl;
					for (int i = 1; i <= this->ReadyQueue.getQueueNum(); i++) {
						cout << this->ReadyQueue.getPCBNode(i)->_value->predictedCount << endl;
					}*/
				}
				else {
					//cout << "error!" << endl;
					this->ReadyQueue.getPCBNode(tmp_counter - 1)->followPointer = midNode->followPointer;
					midNode->followPointer = this->ReadyQueue.getPCBNode(tmp_counter)->followPointer;
					this->ReadyQueue.getPCBNode(tmp_counter)->followPointer = midNode;
					/*cout << this->ReadyQueue.getQueueNum() << endl;
					for (int i = 1; i <= this->ReadyQueue.getQueueNum(); i++) {
						cout << this->ReadyQueue.getPCBNode(i)->_value->predictedCount << endl;
					}*/
					this->ReadyQueue.setEndPtr(this->ReadyQueue.getPCBNode(this->ReadyQueue.getQueueNum()));
				}
			}
		}
	}
	/*cout << this->ReadyQueue.getQueueNum() << endl;
	for (int i = 1; i <= this->ReadyQueue.getQueueNum(); i++) {
		cout << this->ReadyQueue.getPCBNode(i)->_value->predictedCount << endl;
	}*/
	lck.unlock();
}

void ProcessScheduler::ProcessResort_SJF() {
	//First Based On Priority
	//bubble sort
	if (this->ReadyQueue.getQueueNum() <= 1) {
		return;
	}
	shared_ptr<ScheduleQueue::PCB> mid_end_ptr;
	std::unique_lock<std::mutex> lck(ScheduleQueue::readyQueueMutex);
	int bubbleTmp = this->ReadyQueue.getQueueNum();
	//cout << "bubbleSignal:" << bubbleTmp << endl;
	for (int bubbleTmp = this->ReadyQueue.getQueueNum(); bubbleTmp > 1; bubbleTmp--) {
		for (int tmp_counter = 1; tmp_counter < bubbleTmp; tmp_counter++) {
			//cout << tmp_counter;
			if (this->ReadyQueue.getNode(tmp_counter)->priority > this->ReadyQueue.getNode(tmp_counter + 1)->priority
				|| (this->ReadyQueue.getNode(tmp_counter)->predictedCount < this->ReadyQueue.getNode(tmp_counter + 1)->predictedCount)) {
				ScheduleQueue::PCBNode* midNode = this->ReadyQueue.getPCBNode(tmp_counter);
				//cout << "bubbleSignal:" << bubbleTmp << endl;
				if (tmp_counter == 1) {
					//cout << "11111";
					this->ReadyQueue.popProcess();
					midNode->followPointer = this->ReadyQueue.frontPtr()->followPointer;
					if (tmp_counter == 2) {
						midNode->followPointer = NULL;
					}
					this->ReadyQueue.insert_num();
					this->ReadyQueue.frontPtr()->followPointer = midNode;
					/*cout << this->ReadyQueue.getQueueNum() << endl;
					for (int i = 1; i <= this->ReadyQueue.getQueueNum(); i++) {
						cout << this->ReadyQueue.getPCBNode(i)->_value->predictedCount << endl;
					}*/
				}
				else {
					//cout << "error!" << endl;
					this->ReadyQueue.getPCBNode(tmp_counter - 1)->followPointer = midNode->followPointer;
					midNode->followPointer = this->ReadyQueue.getPCBNode(tmp_counter)->followPointer;
					this->ReadyQueue.getPCBNode(tmp_counter)->followPointer = midNode;
					/*cout << this->ReadyQueue.getQueueNum() << endl;
					for (int i = 1; i <= this->ReadyQueue.getQueueNum(); i++) {
						cout << this->ReadyQueue.getPCBNode(i)->_value->predictedCount << endl;
					}*/
					this->ReadyQueue.setEndPtr(this->ReadyQueue.getPCBNode(this->ReadyQueue.getQueueNum()));
				}
			}
		}
	}
	/*cout << this->ReadyQueue.getQueueNum() << endl;
	for (int i = 1; i <= this->ReadyQueue.getQueueNum(); i++) {
		cout << this->ReadyQueue.getPCBNode(i)->_value->predictedCount << endl;
	}*/
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
		//cout << "33333";
		unique_lock conditionLock(_mutex);
		//cout << "22222";
		//_condition.wait(conditionLock, [this] { return _waken.load(); });
		//cout << "11111";
		/*scoped_lock queueLock(
			ScheduleQueue::newlyCreatedQueueMutex, 
			ScheduleQueue::readyQueueMutex,
			ScheduleQueue::waitingQueueMutex
		);*/
		//code here
		//algorithm for scheduling
		/*Have no idea know what should be done here.*/
		this->ProcessSchedule_Preemptive();
		//switch...case...
		

		_waken = false;
	}
}
