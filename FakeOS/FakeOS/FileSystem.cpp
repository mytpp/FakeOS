#include "FileSystem.h"
#include <cassert>
#include <list>
#include <fstream>
#include <iostream>



using namespace std;
enum ftype :uint8_t { kFile, kDirectory };
//map the directory to real file directory in computer
class FileSystem::INode
{
public:
	INode(string name, std::string fpath, ftype type);
	~INode();
	void buildDirectories(fs::path fpath);
	bool addChild(const string& name, const std::string& fpath, const Method& type, const string& content = "") {
		ftype t = type == kCreateFile ? kFile : kDirectory;
		INode* rawchild = new INode(name, fpath, t);
		std::shared_ptr<INode> child(rawchild);
		_children.push_back(child);
		//build file in disk
		return true;
	}
	bool eraseChild(const string& name) {
		for (_itr_node = _children.begin(); _itr_node != _children.end(); _itr_node++) {
			if ((*_itr_node)->_name == name)
				_children.erase(_itr_node);
		}
		return true;
	}
	shared_ptr<INode> getParent() {
		return _parent.lock();
	}
	std::string getPath() {
		return _path;
	}
	bool ifChildreneExist() {
		cout << "here5" << endl;
		bool tmp = _children.empty();
		return tmp;
	}
	std::list<shared_ptr<INode>>::iterator getChildren() {
		return _children.begin();
	}
	std::list<shared_ptr<INode>>::iterator getChildren_end() {
		return _children.end();
	}
	std::vector<std::string> getChildrenname() {
		std::vector<std::string> namelist;
		for (_itr_node = _children.begin(); _itr_node != _children.end(); _itr_node++)
			namelist.push_back((*_itr_node)->getName());
		return namelist;
	}
	std::string getName() {
		return _name;
	}
	void setName(const std::string& name) {
		_name = name;
	}

	//add its and all its forefathers' _lockCount
	void lockNodePath() {
		_lockCount += 1;
		if (_path != "root")
			_parent.lock()->lockNodePath();

	}
	std::atomic<uint8_t> getLockCount() {
		return (unsigned char)_lockCount;
	}

private:
	//if a process in current directory is running, then all nodes 
	//in the process's path are locked, i.e. couldn't be deleted.
	std::atomic<uint8_t> _lockCount;
	std::string _name;
	ftype _type;
	std::string _path;
	weak_ptr<INode> _parent;
	std::list<shared_ptr<INode>> _children;
	std::list<shared_ptr<INode>>::iterator _itr_node;
};

FileSystem::INode::INode(string name, std::string fpath, ftype type)
	:_name(std::move(name)), _path(std::move(fpath)), _type(std::move(type))
{
}

FileSystem::INode::~INode()
{
}

void FileSystem::INode::buildDirectories(fs::path fpath)
{
	/*
		创建的file的name不可以包含'/'
		创建的direcories的name不可以包含'.'和'/'
		使用是否包含'.'来判断文件类型
	*/
	for (auto& itr : fs::directory_iterator(fpath)) {
		auto title = itr.path();
		std::string str_title = title.generic_string();
		int pos = str_title.find_first_of('/') + 1;
		std::string fname = str_title.substr(pos);
		if (fname.find('.') == string::npos) {
			INode* tempchild = new INode(fname, fpath.generic_string(), kDirectory);
			std::shared_ptr<INode> child(tempchild);
			child->buildDirectories(fpath / fname);
			_children.push_back(child);
		}
		else {
			INode* tempchild = new INode(fname, fpath.generic_string(), kFile);
			std::shared_ptr<INode> child(tempchild);
			_children.push_back(child);
		}
		//std::cout << pos << std::endl;
		//std::cout << str_title.substr(pos) << std::endl;
	}
}



FileSystem::FileSystem()
	:_quit(false)
	, _thread(nullptr, kernel::ThreadDeleter)
{
	if (!fs::exists("root")) {
		fs::create_directory("root");
		_absoluteRootPath = fs::path("root");
		_root = std::shared_ptr<INode>(new INode("root", _absoluteRootPath.generic_string(), kDirectory));
		_workingDirectory = _root;
	}
	else {
		_absoluteRootPath = fs::path("root");
		_root = std::shared_ptr<INode>(new INode("root", _absoluteRootPath.generic_string(), kDirectory));
		_workingDirectory = _root;
		_root->buildDirectories(_absoluteRootPath);
	}

}


FileSystem::~FileSystem() = default;


void FileSystem::start()
{
	assert(!_thread);
	_quit = false;

	_thread.reset(new thread(
		bind(&FileSystem::threadFunc, this)
	));
	_condition.notify_all();
}

void FileSystem::quit()
{
	_quit = true;
}

void FileSystem::lockPath()
{

}



std::vector<std::string> FileSystem::list()
{
	if (_workingDirectory->ifChildreneExist())
		cout << "empty!" << endl;
	return _workingDirectory->getChildrenname();
}

std::future<bool> FileSystem::createFile(const std::string& name, const std::string& content)
{
	std::unique_lock<std::mutex> lck(_mutex);
	_condition.wait(lck);
	if (name.find('/') != string::npos) {
		std::future<bool> result = std::async(std::launch::async, []() {
			return false;
			});
		_condition.notify_all();
		return result;
	}

	CreateFileParams param = { name, _workingDirectory->getPath() + "/" + name, content };
	IORequestPacket packet = { kCreateFile, param };
	packet.workingDirectory = std::shared_ptr(_workingDirectory);
	std::promise<bool> proObj;
	_messageQueue.emplace(packet, std::move(proObj));

	_condition.notify_all();
	return proObj.get_future();
}

std::future<bool> FileSystem::createDirectory(const std::string& name)
{
	std::unique_lock<std::mutex> lck(_mutex);
	//_condition.wait(lck);
	cout << "new directoy:" << name << endl;

	CreateDirectoryParams param = { name, _workingDirectory->getPath() + "/" + name };
	IORequestPacket packet = { kMakeDirectory, param };
	packet.workingDirectory = std::shared_ptr(_workingDirectory);
	//packet.workingDirectory = std::enable_shared_from_this<>;
	std::promise<bool> proObj;
	std::future<bool> fuObj = proObj.get_future();
	_messageQueue.emplace(packet, std::move(proObj));
	if (!_messageQueue.empty())
		cout << "not empty" << endl;
	lck.unlock();
	_condition.notify_all();
	
	return fuObj;
}

/*void FileSystem::createDirectory(const std::string& name)
{
	std::unique_lock<std::mutex> lck(_mutex);
	//_condition.wait(lck);
	cout << "new directoy:" << name << endl;
	if (name.find('/') != string::npos && name.find('.') != string::npos) {
		
		return;
	}
	CreateDirectoryParams param = { name, _workingDirectory->getPath() + "/" + name };
	IORequestPacket packet = { kMakeDirectory, param };
	packet.workingDirectory = std::shared_ptr(_workingDirectory);
	std::promise<bool> proObj;
	_messageQueue.emplace(packet, std::move(proObj));
	if (!_messageQueue.empty())
		cout << "not empty" << endl;
	lck.unlock();
	_condition.notify_all();
}*/


std::future<bool> FileSystem::removeFile(const std::string& name)
{
	std::unique_lock<std::mutex> lck(_mutex);
	_condition.wait(lck);
	bool ifexist = false;
	shared_ptr<INode> tempptr;
	for (_itr_node = _workingDirectory->getChildren(); _itr_node != _workingDirectory->getChildren_end(); _itr_node++) {
		if ((*_itr_node)->getName() == name) {
			ifexist = true;
			tempptr = (*_itr_node);
			break;
		}
	}
	if (tempptr->getLockCount() > 0 && !ifexist) {
		std::future<bool> result = std::async(std::launch::async, []() {
			return false;
			});
		_condition.notify_all();
		return result;
	}

	RemoveFileParams param = { name, _workingDirectory->getPath() };
	IORequestPacket packet = { kDeleteFile, param };
	packet.workingDirectory = std::shared_ptr(_workingDirectory);
	std::promise<bool> proObj;
	_messageQueue.emplace(packet, std::move(proObj));

	_condition.notify_all();
	return proObj.get_future();
}

std::future<bool> FileSystem::rename(const std::string& oldname, const std::string& newname)
{
	std::unique_lock<std::mutex> lck(_mutex);
	_condition.wait(lck);
	bool ifexist = false;
	shared_ptr<INode> tempptr;
	for (_itr_node = _workingDirectory->getChildren(); _itr_node != _workingDirectory->getChildren_end(); _itr_node++) {
		if ((*_itr_node)->getName() == oldname) {
			ifexist = true;
			tempptr = (*_itr_node);
			break;
		}
	}
	if (tempptr->getLockCount() > 0 && !ifexist) {
		std::future<bool> result = std::async(std::launch::async, []() {
			return false;
			});
		_condition.notify_all();
		return result;
	}
	RenameParams param = { oldname, newname, _workingDirectory->getPath() };
	IORequestPacket packet = { kRename, param };
	packet.workingDirectory = std::shared_ptr(_workingDirectory);
	std::promise<bool> proObj;
	_messageQueue.emplace(packet, std::move(proObj));

	_condition.notify_all();
	return proObj.get_future();
}

std::future<bool> FileSystem::back()
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

std::future<bool> FileSystem::load(const string& name)
{
	std::unique_lock<std::mutex> lck(_mutex);
	_condition.wait(lck);

	bool ifexist = false;
	for (_itr_node = _workingDirectory->getChildren(); _itr_node != _workingDirectory->getChildren_end(); _itr_node++) {
		if ((*_itr_node)->getName() == name) {
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

std::string FileSystem::nowpath()
{
	return _workingDirectory->getPath();
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
			//cout << _messageQueue.front().first.method << endl;

		
			if (_messageQueue.front().first.method == kCreateFile) {
				CreateFileParams param = std::get<CreateFileParams>(_messageQueue.front().first.params);
				_itr_node = _messageQueue.front().first.workingDirectory->getChildren();
				bool ifexist = false;
				for ( ; _itr_node != _messageQueue.front().first.workingDirectory->getChildren_end(); _itr_node++) {
					if ((*_itr_node)->getName() == param.name) {
						_messageQueue.front().second.set_value(false);
						ifexist = true;
						break;
					}
				}
				if (!ifexist) {
					_messageQueue.front().first.workingDirectory->addChild(param.name, param.fpath, _messageQueue.front().first.method, param.content);
					_messageQueue.front().second.set_value(true);
					std::ofstream(param.fpath);
				}
			}
			else if (_messageQueue.front().first.method == kDeleteFile) {
				RemoveFileParams param = std::get<RemoveFileParams>(_messageQueue.front().first.params);
				_itr_node = _messageQueue.front().first.workingDirectory->getChildren();
				bool ifdel = true;
				for (; _itr_node != _messageQueue.front().first.workingDirectory->getChildren_end(); _itr_node++) {
					if ((*_itr_node)->getName() == param.name) {
						_messageQueue.front().first.workingDirectory->eraseChild(param.name);
						_messageQueue.front().second.set_value(true);
						ifdel = false;
						fs::remove_all(param.fpath);
						break;
					}
				}
				if (ifdel)
					_messageQueue.front().second.set_value(false);
			}
			else if (_messageQueue.front().first.method == kMakeDirectory) {

				cout << "here1" << endl;
				CreateDirectoryParams param = std::get<CreateDirectoryParams>(_messageQueue.front().first.params);

				bool ifexist = false;

				_workingDirectory->ifChildreneExist();
				cout << "here3" << endl;
				_messageQueue.front().first.workingDirectory->ifChildreneExist();
				cout << "here4" << endl;
				if (!_messageQueue.front().first.workingDirectory->ifChildreneExist()) {
					_itr_node = _messageQueue.front().first.workingDirectory->getChildren();
					cout << "here3" << endl;
					for (; _itr_node != _messageQueue.front().first.workingDirectory->getChildren_end(); _itr_node++) {
						if ((*_itr_node)->getName() == param.name) {
							cout << "no setvalue" << endl;
							_messageQueue.front().second.set_value(false);
							cout << "setvalue" << endl;
							ifexist = true;
							break;
						}
					}
				}
				if (!ifexist) {
					_messageQueue.front().first.workingDirectory->addChild(param.name, param.fpath, _messageQueue.front().first.method);
					_messageQueue.front().second.set_value(true);
					fs::create_directories(param.fpath);
				}
				cout << "here2" << endl;
			}
			else if (_messageQueue.front().first.method == kRename) {
				RenameParams param = std::get<RenameParams>(_messageQueue.front().first.params);
				_itr_node = _messageQueue.front().first.workingDirectory->getChildren();
				bool ifexist = false;
				bool ifrepeat = false;
				for ( ; _itr_node != _messageQueue.front().first.workingDirectory->getChildren_end(); _itr_node++) {
					if ((*_itr_node)->getName() == param.oldname)
						ifexist = true;
					if ((*_itr_node)->getName() == param.newname)
						ifrepeat = true;
				}
				if (ifexist && !ifrepeat) {
					(*_itr_node)->setName(param.newname);
					fs::rename(fs::path(param.fpath) / param.oldname, fs::path(param.fpath) / param.newname);
				}
				else
					_messageQueue.front().second.set_value(false);
			}
			_messageQueue.pop();
			_condition.notify_all();
		}

		//process request


		//if(success)
		//request.second.set_value(true);
		//else
		//request.second.set_value(false);
	}
}
