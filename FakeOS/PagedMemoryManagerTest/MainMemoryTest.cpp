#include "../FakeOS/PagedMemoryManager.h"
#include "../FakeOS/ScheduleQueue.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace kernel;
using namespace ScheduleQueue;

shared_ptr<PCB> loadProcess(const string& filename)
{
	static uint16_t nextPid = 0;
	shared_ptr<PCB> pcb = make_shared<PCB>();
	pcb->pid = nextPid++;
	pcb->pageTable = make_unique<PageTable>();
	for (auto& pte : *(pcb->pageTable))
	{
		pte.free = true;
	}
	ifstream executable(filename, ios::in | ios::ate);
	auto size = executable.tellg();
	pcb->textSegment.assign(size, '\0');
	executable.seekg(0);
	executable.read(pcb->textSegment.data(), size);
	return pcb;
}

void parseCommand(istringstream& command, shared_ptr<PCB> pcb)
{
	string op;
	string var;
	string null; //store rubbish
	size_t size;
	size_t start;
	size_t offset;
	bool succ;
	command >> op;
	if (op == "dm")
	{
		command >> size >> null >> var;
		succ = memoryManager->virtualAllocate(pcb, size, start);
		cout << "Allocating Memory Succeed: " << succ << endl;
		pcb->allocatedMemory[var] = start;
		memoryManager->printMemoryStatistics();
	}
	else if (op == "am")
	{
		command >> var >> offset;
		succ = memoryManager->accessMemory(pcb, pcb->allocatedMemory[var] + offset);
		cout << "Accessing Memory Succeed: " << succ << endl;
		memoryManager->printMemoryStatistics();
	}
	else if (op == "fm")
	{
		command >> var;
		memoryManager->printPagedMemoryInProcess(pcb);
		succ = memoryManager->virtualFree(pcb, pcb->allocatedMemory[var]);
		cout << "Freeing Memory Succeed: " << succ << endl;
		memoryManager->printMemoryStatistics();
	}
	else
	{
		cout << "Unrecognized command: " << op << endl;
	}
	memoryManager->printPagedMemoryInProcess(pcb);
}

void testSingle(shared_ptr<PCB> pcb)
{
	cout << "New test start." << endl;
	string cmd;
	istringstream text(pcb->textSegment);
	cout << boolalpha;
	while (getline(text, cmd))
	{
		if (cmd.empty())
			continue;
		cin.get(); //pause
		cout << "command: " << cmd << endl;
		istringstream command(cmd);
		parseCommand(command, pcb);
	}
}

void testMultiple(const string& filename)
{
	string cmd;
	bool succ;
	uint16_t pid;
	map<uint16_t, shared_ptr<PCB>> pcbs;
	ifstream executable(filename);
	stringstream text;
	text << executable.rdbuf();
	executable.close();

	cout << boolalpha;
	while (getline(text, cmd))
	{
		if (cmd.empty())
			continue;
		cin.get(); //pause
		cout << "command: " << cmd << endl;
		istringstream command(cmd);
		command >> pid;
		if (pcbs.find(pid) == pcbs.end())
		{
			pcbs[pid] = make_shared<PCB>();
			pcbs[pid]->pid = pid;
			pcbs[pid]->pageTable = make_unique<PageTable>();
			for (auto& pte : *(pcbs[pid]->pageTable))
			{
				pte.free = true;
			}
		}
		parseCommand(command, pcbs[pid]);
	}
}

int main()
{
	cout << "Start testing PagedMemoryManager..." << endl;
	memoryManager = make_unique<PagedMemoryManager>();
	cout << "Memory allocator loaded." << endl;

	//testSingle(loadProcess("p1.txt"));
	testMultiple("p2.txt");

	system("pause");
}
