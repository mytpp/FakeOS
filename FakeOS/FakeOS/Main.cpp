#include "Kernel.h"
#include "CPUCore.h"
#include "ProcessScheduler.h"
#include "PagedMemoryManager.h"
#include "FileSystem.h"
//#include "Command.h"
#include <iostream>
#include <string_view>

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

int main() 
{
	cout << "Loading kernel..." << endl;

	fileSystem = make_unique<FileSystem>();
	fileSystem->start();
	cout << "File system loaded." << endl;

	processScheduler = make_unique<ProcessScheduler>();
	processScheduler->start();
	cout << "Process scheduler loaded." << endl;

	memoryManager = make_unique<PagedMemoryManager>();
	cout << "Memory allocator loaded." << endl;

	cpuCore = make_unique<CPUCore>();
	cpuCore->start();
	cout << "Process simulator loaded." << endl;


	//After initialization finished, main thread runs as cmd parser.
	string command;
	while (true)
	{
		cout << ">>"; //command prompt
		getline(cin, command);
		//ExplainCmd(cmd);

		//parse command and execute
		string_view command_view(command);
		removePrefixedSpace(command_view);
		int separatorPosition = command_view.find_first_of(' ');
		string_view commandName = command_view.substr(0, separatorPosition);
		
		if (commandName == "ls")
		{
			cout << "ls" << endl; break;
			auto names = fileSystem->list();
			for (auto& name : names)
				cout << name << endl;
		}
		else if (commandName == "cd") 
		{
			string_view path = getNextParameter(command_view, separatorPosition);
			if (command_view == "")
				cout << "fileSystem->changeDirectory(string{path});" << endl;
			else
				cout << "Unrecognized Parameters" << endl;
		}
		else if (commandName == "cf")
		{
			string_view name = getNextParameter(command_view, separatorPosition);
			if (command_view.empty() || (command_view.size() >= 2 &&
				command_view.front() == '"' && command_view.back() == '"'))
				cout << "cf" << endl;
				/*fileSystem->createFile(
					string{ name },
					string{ command_view });*/
			else
				cout << "Unrecognized Parameters" << endl;
		}
		else if (commandName == "af")
		{
			string_view name = getNextParameter(command_view, separatorPosition);
			if(command_view.size() >= 2 &&
				command_view.front() == '"' && command_view.back() == '"')
				cout << "af" << endl;
				/*fileSystem->appendFile(
					string{ name },
					string{ command_view });*/
			else
				cout << "Unrecognized Parameters" << endl;
		}
		else if (commandName == "md")
		{
			string_view name = getNextParameter(command_view, separatorPosition);
			if (command_view == "")
				cout << "md" << endl;
				//fileSystem->createDirectory(string{ name });
			else
				cout << "Unrecognized Parameters" << endl;
		}
		else if (commandName == "rm")
		{
			string_view name = getNextParameter(command_view, separatorPosition);
			if (command_view == "")
				cout << "rm" << endl;
				//fileSystem->createDirectory(string{ name });
			else
				cout << "Unrecognized Parameters" << endl;
		}
		else if (commandName._Starts_with("./"))
		{
			string_view programName = commandName.substr(2);
			ScheduleQueue::LoadProcess(string{ programName });
		}
		else if (commandName == "q")
		{
			cout << "Closing..." << endl;
			break;
		}
		cout << endl;
	}

	fileSystem->quit();
	cpuCore->quit(); //must be closed before processScheduler
	processScheduler->quit();
}
