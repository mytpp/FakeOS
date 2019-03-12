#include "FileSystem.h"
#include <cassert>
#include <list>

using namespace std;


//map the directory to real file directory in computer
class FileSystem::INode
{
public:
	INode(string name);
	~INode();

	bool addChild(const string& name);
	bool eraseChild(const string& name);

	//add its and all its forefathers' _lockCount
	void lock();

private:
	enum :uint8_t { kFile, kDirectory } _type;
	//if a process in current directory is running, then all nodes 
	//in the process's path are locked, i.e. couldn't be deleted.
	std::atomic<uint8_t> _lockCount; 
	string _name;
	unique_ptr<INode> _parent;
	list<unique_ptr<INode>> _children;
};

FileSystem::INode::INode(string name)
	:_name(std::move(name))
{
}

FileSystem::INode::~INode()
{
}



FileSystem::FileSystem()
	:_quit(false)
	, _thread(nullptr, kernel::ThreadDeleter)
{
}

FileSystem::~FileSystem() = default;


void FileSystem::start()
{
	assert(!_thread);
	_quit = false;
	_thread.reset(new thread(
		bind(&FileSystem::threadFunc, this)
	));
}

void FileSystem::quit()
{
	_quit = true;
}

void FileSystem::threadFunc()
{
	while (!_quit)
	{
		std::pair<IORequestPacket, std::promise<bool>> request;

		//retrive a new IO request, sleep if there is no IO request
		{ //critical section, better make it smaller
			unique_lock<std::mutex> lock(_mutex);
			_condition.wait(lock, [this] { return !_messageQueue.empty(); });
			request = std::move(_messageQueue.front());
			_messageQueue.pop();
		}

		//process request


		//if(success)
		//request.second.set_value(true);
		//else
		//request.second.set_value(false);
	}
}
