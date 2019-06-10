#include "CPUCore.h"
#include "ProcessScheduler.h"
#include "MemoryManager.h"
#include "FileSystem.h"
#include <cassert>
#include <iostream>
#include <sstream>

using namespace std;
using namespace kernel;



inline void removePrefixedSpace(string_view& sv)
{
	sv.remove_prefix(min(sv.find_first_not_of(' '), sv.size()));
}

inline string_view getNextParameter(string_view& sv, int& pos)
{
	sv = sv.substr(pos + 1);
	removePrefixedSpace(sv);
	pos = sv.find_first_of(' ');
	string_view ret = sv.substr(0, pos);
	removePrefixedSpace(sv);
	return ret;
}


CPUCore::CPUCore()
	:_quit(false)
	, _thread(nullptr, kernel::ThreadDeleter)
	, _timeElapsed(0)
{
}

CPUCore::~CPUCore()
{
}

void CPUCore::start()
{
	assert(!_thread);
	_quit = false;
	_thread.reset(new thread(
		bind(&CPUCore::threadFunc, this)
	));
}

void CPUCore::quit()
{
	_quit = true;
}

void CPUCore::threadFunc()
{
	while (!_quit)
	{
		//periodically wake up processScheduler
		if (_timeElapsed % kScheduleInterval == 0)
			processScheduler->wakeUp();

		{//critical section
			//scoped_lock lock(ScheduleQueue::readyQueueMutex);
			if (processScheduler->getRunningProcess() == NULL) {
				return;
			}
			else {
				processScheduler->getRunningProcess()->programCounter--;
				if (processScheduler->getRunningProcess()->programCounter == 0) {
					string tmpStr = processScheduler->getRunningProcess()->restCode.substr(2, processScheduler->getRunningProcess()->restCode.find('\n'));
					if (processScheduler->getRunningProcess()->restCode.find('\n') == processScheduler->getRunningProcess()->restCode.npos ||
						processScheduler->getRunningProcess()->restCode.find('\n') == processScheduler->getRunningProcess()->restCode.length() - 1) {
						processScheduler->getRunningProcess()->state = ScheduleQueue::kTerminated;
						/*All demands have been done, so the process should be terminated*/
					}
					else {
						int index = processScheduler->getRunningProcess()->restCode.find('\n');
						processScheduler->getRunningProcess()->restCode = processScheduler->getRunningProcess()->restCode.substr(index + 1, processScheduler->getRunningProcess()->restCode.length() - 1);
						processScheduler->getRunningProcess()->programCounter = processScheduler->getRunningProcess()->restCode.at(0);
					}
					processScheduler->getRunningProcess()->state = ScheduleQueue::kReady;
					ParseAndDoDirective(tmpStr, processScheduler->getRunningProcess()->file_ptr);
				}
			}
			//peek the top element of kernel::readyQueue
			//if programCounter reach 0, call ParseAndDoDirective()
		}

		{//critical section
			//scoped_lock lock(ScheduleQueue::waitingQueueMutex);
			for (int ioCounter = 0; ioCounter < _ioChannel.size(); ioCounter++)
			{
				if (_ioChannel[ioCounter].event.wait_for(std::chrono::milliseconds(5)) == std::future_status::ready) {
					_ioChannel[ioCounter].pcb->state = ScheduleQueue::kReady;
				}
			}
			//poll _ioChannel to see if there is IO event ready,
			//and change the corresponding PCB's state to ready

		}

		this_thread::sleep_for(kernel::kCPUCycle);
		_timeElapsed++;
	}
}

bool CPUCore::ParseAndDoDirective(const string& directive, uint16_t file_ptr)
{
	//interact with MemoryAllocator and FileSystem
	//code here...
	//stringstream ss(directive);
	//string word;
	//int len = directive.length();
	//int numind = directive.find(' ');
	//if (numind == string::npos)
	//	return false;
	//int time=atoi(directive.substr(0, numind).c_str());
	//if (!(ss >> word))return false;
	//int time = atoi(word.c_str());
	//if (time < 0)
	//	return false;
	//if (!(ss >> word))return false;
	//string curd_d = word;
	//if (curd_d == "DemandMemory")
	//{
	//	if (!(ss >> word))return false;
	//	int var1 = atoi(word.c_str());
	//	if (!(ss >> word) || word != "As")return false;
	//	if (!(ss >> word))return false;
	//	int var2 = atoi(word.c_str());
	//	// process
	//	//memoryManager->virtualAllocate(_ioChannel.pcb, var1, var2);
	//}
	//else if (curd_d == "FreeMemory")
	//{
	//	if (!(ss >> word))return false;
	//	int var1 = atoi(word.c_str());
	//	// process
	//	//memoryManager->virtualFree(_ioChannel.pcb, var1);
	//}
	//else if (curd_d == "AccessMemory")
	//{
	//	if (!(ss >> word))return false;
	//	int var1 = atoi(word.c_str());
	//	// process
	//	//memoryManager->accessMemory(_ioChannel.pcb, var1);
	//}
	//else if (curd_d == "CreateFile")
	//{
	//	if (!(ss >> word))return false;
	//	string filename = word;
	//	int ind = directive.find(word);
	//	string content = directive.substr(ind + word.length());
	//	//process
	//	fileSystem->createFile(filename, content);
	//}
	//else if (curd_d == "DeleteFile")
	//{
	//	if (!(ss >> word))return false;
	//	string filename = word;
	//	//process
	//	fileSystem->removeFile(filename);
	//}
	//else if (curd_d == "DeleteFile")
	//{
	//	cout << "ls" << endl;
	//	auto names = fileSystem->list();
	//	for (auto& name : names)
	//		cout << name << endl;
	//}
	//else if (curd_d == "cd")
	//{
	//	if (!(ss >> word))return false;
	//	string path = word;

	//	if (path == "..")
	//		fileSystem->back();
	//	else {
	//		cout << "loading..." << endl;
	//		fileSystem->load(path);
	//	}


	//}
	string_view command_view(directive);
	removePrefixedSpace(command_view);
	int separatorPosition = command_view.find_first_of(' ');
	string_view commandName = command_view.substr(0, separatorPosition);


	if (commandName == "DemandMemory")
	{
		size_t var1, var2;
		string_view word = getNextParameter(command_view, separatorPosition);
		if (command_view != "") {
			var1 = atoi(string{ word }.c_str());
		}
		else
			cout << "Unrecognized Parameters" << endl;


		separatorPosition = command_view.find_first_of(' ');
		string_view middle = getNextParameter(command_view, separatorPosition);
		if (command_view == "" || string{ middle } == "As")
			cout << "Unrecognized Parameters" << endl;

		separatorPosition = command_view.find_first_of(' ');
		string_view word2 = getNextParameter(command_view, separatorPosition);
		if (command_view != "" && string{ middle } == "As") {
			var2 = atoi(string{ word2 }.c_str());
		}
		else
			cout << "Unrecognized Parameters" << endl;

		// process
		memoryManager->virtualAllocate(processScheduler->getRunningProcess(), var1, var2);
	}
	else if (commandName == "FreeMemory")
	{
		string_view word = getNextParameter(command_view, separatorPosition);
		size_t var1 = atoi(string{ word }.c_str());
		// process
		memoryManager->virtualFree(processScheduler->getRunningProcess(), var1);
	}
	else if (commandName == "AccessMemory")
	{
		string_view word = getNextParameter(command_view, separatorPosition);
		size_t var1 = atoi(string{ word }.c_str());
		// process
		memoryManager->accessMemory(processScheduler->getRunningProcess(), var1);

	}
	else if (commandName == "ls")
	{
		auto names = fileSystem->list(file_ptr);
		for (auto& name : names)
			cout << name << endl;
	}
	else if (commandName == "cd")
	{
		string_view path = getNextParameter(command_view, separatorPosition);
		if (command_view != "") {
			if (string{ path } == "..")
				fileSystem->back(file_ptr);
			else {
				cout << "loading..." << endl;
				fileSystem->load(string{ path }, file_ptr);
			}
		}
		else
			cout << "Unrecognized Parameters" << endl;
	}
	else if (commandName == "cf")
	{
		string_view name = getNextParameter(command_view, separatorPosition);
		separatorPosition = command_view.find_first_of(' ');
		string_view content = getNextParameter(command_view, separatorPosition);

		if (command_view.size() >= 2 &&
			command_view.front() == '"' && command_view.back() == '"') {
			size_t pos1 = command_view.find_first_of('"');
			size_t pos2 = command_view.find_last_of('"');
			command_view = command_view.substr(pos1 + 1, pos2 - 1);

			fileSystem->createFile(
				string{ name },
				string{ command_view }, file_ptr);
		}
		else
			cout << "Unrecognized Parameters" << endl;
	}
	else if (commandName == "af")
	{
		string_view name = getNextParameter(command_view, separatorPosition);
		separatorPosition = command_view.find_first_of(' ');
		string_view content = getNextParameter(command_view, separatorPosition);

		if (command_view.size() >= 2 &&
			command_view.front() == '"' && command_view.back() == '"') {
			size_t pos1 = command_view.find_first_of('"');
			size_t pos2 = command_view.find_last_of('"');
			cout << pos1 << " " << pos2 << endl;
			command_view = command_view.substr(pos1 + 1, pos2 - 1);
			cout << string{ command_view } << endl;

			fileSystem->appendFile(
				string{ name },
				string{ command_view }, file_ptr);
		}
		else
			cout << "Unrecognized Parameters" << endl;
	}
	else if (commandName == "md")
	{
		string_view name = getNextParameter(command_view, separatorPosition);
		if (command_view != "") {
			fileSystem->createDirectory(string{ name }, file_ptr);
		}
		else
			cout << "Unrecognized Parameters" << endl;
	}
	else if (commandName == "rm")
	{
		string_view name = getNextParameter(command_view, separatorPosition);
		if (command_view != "") {
			fileSystem->removeFile(string{ name }, file_ptr);
		}
		else
			cout << "Unrecognized Parameters" << endl;
	}
	else if (commandName._Starts_with("./"))
	{
		string_view programName = commandName.substr(2);
		ScheduleQueue::LoadProcess(string{ programName }, file_ptr);
	}
	else
		return false;//if directive is unrecognizable

	return true;
}



