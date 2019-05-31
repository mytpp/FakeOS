#include "ProcessScheduler.h"
#include "ScheduleQueue.h"
#include <cassert>

using namespace std;




ProcessScheduler::ProcessScheduler(Method method)
	:_scheduleMethod(method)
	,_waken(false)
	,_quit(false)
	,_thread(nullptr, kernel::ThreadDeleter)
{
}
void ProcessScheduler::ProcessSchedule() {
	if (this->ReadyQueue.front() == NULL) {
		return;
	}
	if (this->ReadyQueue.front()->state == ScheduleQueue::kRunning) {
		this->ReadyQueue.front()->state = ScheduleQueue::kReady;
	}
	if (this->ReadyQueue.front()->state == ScheduleQueue::kTerminated) {
		//giving a log for end of the Process?
		for (int newlyCreatedCount = 0; newlyCreatedCount < NewlyCreatedQueue.getQueueNum(); newlyCreatedCount++) {
			this->ReadyQueue.addProcess(this->NewlyCreatedQueue.remove_Node(newlyCreatedCount)->_value);
			this->ReadyQueue.insert_num();
		}
		this->ReadyQueue.popProcess();
		ProcessResort_FCFS();
	}
	else if (this->ReadyQueue.front()->state == ScheduleQueue::kWaiting) {
		for (int newlyCreatedCount = 0; newlyCreatedCount < NewlyCreatedQueue.getQueueNum(); newlyCreatedCount++) {
			this->ReadyQueue.addProcess(this->NewlyCreatedQueue.remove_Node(newlyCreatedCount)->_value);
			this->ReadyQueue.insert_num();
		}
		for (int waitCount = 0; waitCount < this->WaitingQueue.getQueueNum(); waitCount++) {
			ScheduleQueue::PCBNode* midPtr = this->WaitingQueue.getPCBNode(waitCount);
			if (midPtr != NULL) {
				if (midPtr->_value->state == ScheduleQueue::kReady) {
					ScheduleQueue::PCBNode* newNode = this->WaitingQueue.remove_Node(waitCount);
					this->ReadyQueue.addProcess(newNode->_value);
					this->ReadyQueue.insert_num();
				}
			}
		}
		this->WaitingQueue.addProcess(this->ReadyQueue.popProcess());
		ProcessResort_FCFS();
	}
	else if (this->ReadyQueue.front()->state == ScheduleQueue::kReady) {
		for (int newlyCreatedCount = 0; newlyCreatedCount < NewlyCreatedQueue.getQueueNum(); newlyCreatedCount++) {
			this->ReadyQueue.addProcess(this->NewlyCreatedQueue.remove_Node(newlyCreatedCount)->_value);
			this->ReadyQueue.insert_num();
		}
		this->ReadyQueue.addProcess(this->ReadyQueue.popProcess());
		//sort the PCB by ComingTime
		ProcessResort_FCFS();
	}
}

void ProcessScheduler::ProcessResort_FCFS() {
	//First Based On Priority
	//bubble sort
	shared_ptr<ScheduleQueue::PCB> mid_end_ptr;
	int bubbleTmp = this->ReadyQueue.getQueueNum();
	for (int bubbleTmp = this->ReadyQueue.getQueueNum(); bubbleTmp > 0; bubbleTmp--) {
		for (int tmp_counter = 0; tmp_counter < bubbleTmp - 1; tmp_counter++) {
			if (this->ReadyQueue.getNode(tmp_counter)->priority < this->ReadyQueue.getNode(tmp_counter + 1)->priority) {
				ScheduleQueue::PCBNode* midNode = this->ReadyQueue.getPCBNode(tmp_counter);
				if (tmp_counter == 0) {
					this->ReadyQueue.popProcess();
					midNode->followPointer = this->ReadyQueue.frontPtr()->followPointer;
					this->ReadyQueue.frontPtr()->followPointer = midNode;
					this->ReadyQueue.insert_num();
				}
				else {
					this->ReadyQueue.getPCBNode(tmp_counter - 1)->followPointer = midNode->followPointer;
					midNode->followPointer = this->ReadyQueue.getPCBNode(tmp_counter + 1);
					this->ReadyQueue.getPCBNode(tmp_counter + 1)->followPointer = midNode;
					this->ReadyQueue.insert_num();
				}
			}
			else if (this->ReadyQueue.getNode(tmp_counter)->statistics.timeCreated > this->ReadyQueue.getNode(tmp_counter + 1)->statistics.timeCreated) {
				ScheduleQueue::PCBNode* midNode = this->ReadyQueue.getPCBNode(tmp_counter);
				if (tmp_counter == 0) {
					this->ReadyQueue.popProcess();
					midNode->followPointer = this->ReadyQueue.frontPtr()->followPointer;
					this->ReadyQueue.frontPtr()->followPointer = midNode;
					this->ReadyQueue.insert_num();
				}
				else {
					this->ReadyQueue.getPCBNode(tmp_counter - 1)->followPointer = midNode->followPointer;
					midNode->followPointer = this->ReadyQueue.getPCBNode(tmp_counter + 1);
					this->ReadyQueue.getPCBNode(tmp_counter + 1)->followPointer = midNode;
					this->ReadyQueue.insert_num();
				}
			}
		}
	}
}

void ProcessScheduler::ProcessResort_TimeSlice() {

}

void ProcessScheduler::CreateProcess(shared_ptr<ScheduleQueue::PCB> newProcess) {
	this->NewlyCreatedQueue.addProcess(newProcess);
}

ProcessScheduler::~ProcessScheduler()
{
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
		//switch...case...
		

		_waken = false;
	}
}
