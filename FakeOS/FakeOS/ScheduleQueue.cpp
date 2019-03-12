#include "ScheduleQueue.h"

using namespace std;

namespace ScheduleQueue
{
	mutex newlyCreatedQueueMutex;
	mutex readyQueueMutex;
	mutex waitingQueueMutex;

	void LoadProcess(const string& path, Priority priority)
	{
		//create PCB
		//put PCB in newlyCreatedQueueMutex
	}
}
