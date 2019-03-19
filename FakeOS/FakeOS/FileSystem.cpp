#include "FileSystem.h"
#include <cassert>
#include <list>

using namespace std;

//map the directory to real file directory in computer
class FileSystem::INode
{
public:
	enum ftype :uint8_t { kFile, kDirectory } _type;
	INode(string name, string path, ftype type);
	~INode();

	bool addChild(const string& name, const string& path, const Method& type, const string& content = "") {
		ftype t = type==kCreateFile ? kFile : kDirectory;
		INode *rawchild = new INode(name, path, t);
		std::shared_ptr<INode> child(rawchild);
		_children.push_back(child);
		//build file in disk
	}
	bool eraseChild(const string& name) {
		for (_itr_node = _children.begin(); _itr_node != _children.end(); _itr_node++) {
			if ((*_itr_node)->_name == name) 
				_children.erase(_itr_node);
		}
	}
	shared_ptr<INode> getParent() {
		return _parent.lock();
	}
	string getPath() {
		return _path;
	}
	list<shared_ptr<INode>> getChildren() {
		return _children;
	}
	string getname() {
		return _name
	}


	//add its and all its forefathers' _lockCount
	void lock();

private:
	//if a process in current directory is running, then all nodes 
	//in the process's path are locked, i.e. couldn't be deleted.
	std::atomic<uint8_t> _lockCount; 
	string _name;
	string _path;
	weak_ptr<INode> _parent;
	list<shared_ptr<INode>> _children;
	list<shared_ptr<INode>>::iterator _itr_node;
};

FileSystem::INode::INode(string name, string path, ftype type)
	:_name(std::move(name)), _path(std::move(path)), _type(std::move(type))
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

std::future<bool> FileSystem::createFile(const std::string & name, const std::string & path, const std::string & content)
{
	std::unique_lock<std::mutex> lck(_mutex);
	_condition.wait(lck);

	CreateFileParams param = { name, _workingDirectory->getPath() + "/" + name, content };
	IORequestPacket packet = { kCreateFile, param };
	packet.workingDirectory = std::shared_ptr(_workingDirectory);
	std::promise<bool> proObj;
	auto reqpack = make_pair(packet, proObj);
	_messageQueue.push(reqpack);

	_condition.notify_all();
	return proObj.get_future();
}

std::future<bool> FileSystem::createDirectory(const std::string & name, const std::string & path)
{
	std::unique_lock<std::mutex> lck(_mutex);
	_condition.wait(lck);

	CreateFileParams param = { name, _workingDirectory->getPath() + "/" + name };
	IORequestPacket packet = { kMakeDirectory, param };
	packet.workingDirectory = std::shared_ptr(_workingDirectory);
	std::promise<bool> proObj;
	auto reqpack = make_pair(packet, proObj);
	_messageQueue.push(reqpack);

	_condition.notify_all();
	return proObj.get_future();
}

std::future<bool> FileSystem::removeFile(const std::string & name, const std::string & path)
{
	std::unique_lock<std::mutex> lck(_mutex);
	_condition.wait(lck);

	CreateFileParams param = { name, _workingDirectory->getPath() };
	IORequestPacket packet = { kDeleteFile, param };
	packet.workingDirectory = std::shared_ptr(_workingDirectory);
	std::promise<bool> proObj;
	auto reqpack = make_pair(packet, proObj);
	_messageQueue.push(reqpack);
	
	_condition.notify_all();
	return proObj.get_future();
}

std::future<bool> FileSystem::demand_back()
{
	std::unique_lock<std::mutex> lck(_mutex);
	_condition.wait(lck);

	bool ifback;
	if (_workingDirectory != _root) {
		_workingDirectory = _workingDirectory->getParent();
		ifback = true;
	}
	else
		ifback = false;
	std::future<bool> result = std::async(std::launch::async, [ifback]() {
		return ifback;
	});
	_condition.notify_all();
	return result;
}

std::future<bool> FileSystem::demand_cd(const string &name)
{
	std::unique_lock<std::mutex> lck(_mutex);
	bool ifexist = false;
	for (_itr_node = _workingDirectory->getChildren().begin(); _itr_node != _workingDirectory->getChildren().end(); _itr_node++) {
		if ((*_itr_node)->getname == name) {
			ifexist = true;
			_workingDirectory = (*_itr_node);
		}
	}
	std::future<bool> result = std::async(std::launch::async, [ifexist]() {
		return ifexist;
	});
	_condition.notify_all();
	return result;
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
			IORequestPacket op = request.first;
			

			if (kCreateFile) {
				CreateFileParams param = std::get<CreateFileParams>(op.params);
				list<shared_ptr<INode>> dict = op.workingDirectory->getChildren();
				bool ifexist = false;
				for (_itr_node = dict.begin(); _itr_node != dict.end(); _itr_node++) {
					if ((*_itr_node)->getname == param.name) {
						request.second.set_value(false);
						ifexist = true;
						break;
					}
				}
				if (!ifexist) {
					op.workingDirectory->addChild(param.name, param.path, op.method, param.content);
					request.second.set_value(true);
				}
			}
			else if (kDeleteFile) {
				RemoveFileParams param = std::get<RemoveFileParams>(op.params);
				list<shared_ptr<INode>> dict = op.workingDirectory->getChildren();
				bool ifdel = true;
				for (_itr_node = dict.begin(); _itr_node != dict.end(); _itr_node++) {
					if ((*_itr_node)->getname == param.name) {
						op.workingDirectory->eraseChild(param.name);
						request.second.set_value(true);
						ifdel = false;
						break;
					}
				}
				if (ifdel) 
					request.second.set_value(false);
			}
			else if (kMakeDirectory) {
				CreateDirectoryParams param = std::get<CreateDirectoryParams>(op.params);
				list<shared_ptr<INode>> dict = op.workingDirectory->getChildren();
				bool ifexist = false;
				for (_itr_node = dict.begin(); _itr_node != dict.end(); _itr_node++) {
					if ((*_itr_node)->getname == param.name) {
						request.second.set_value(false);
						ifexist = true;
						break;
					}
				}
				if (!ifexist) {
					op.workingDirectory->addChild(param.name, param.path, op.method);
					request.second.set_value(true);
				}
			}
			
			_messageQueue.pop();
		}

		//process request


		//if(success)
		//request.second.set_value(true);
		//else
		//request.second.set_value(false);
	}
}
