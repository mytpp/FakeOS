#include "FileSystem.h"
#include <cassert>
#include <list>
#include <fstream>
#include <iostream>

/*待修改：
将判断io请求是否成功放入请求处理队列中
对锁定路径进行修改*/

using namespace std;
enum ftype :uint8_t { kFile, kDirectory };
//map the directory to real file directory in computer

class FileSystem::INode
{
public:
	INode(string name, std::string fpath, ftype type, std::string content);
	
	~INode();
	void buildDirectories(fs::path fpath, std::shared_ptr<INode> pa);
	bool addChild(const string& name, const std::string& fpath, const Method& type, std::shared_ptr<INode> pa, const string& content = "");
	bool eraseChild(const string& name);

	shared_ptr<INode> getParent() {
		return _parent.lock();
	}
	std::string getPath() {
		return _path;
	}
	std::string getContent() {
		return _content;
	}
	bool ifChildreneExist() {
		bool tmp = !_children.empty();
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
		for (_itr_node = _children.begin(); _itr_node != _children.end(); _itr_node++) {
			namelist.push_back((*_itr_node)->getName());
		}
		return namelist;
	}
	std::string getName() {
		return _name;
	}
	void setName(const std::string& name) {
		_name = name;
	}
	bool iftxt() {
		return _type == kFile;
	}
	void append(std::string apc) {
		_content += apc;
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

	std::string _content;
	std::string _path;
	weak_ptr<INode> _parent;
	std::list<shared_ptr<INode>> _children;
	std::list<shared_ptr<INode>>::iterator _itr_node;
};

FileSystem::INode::INode(string name, std::string fpath, ftype type, std::string content="")
	:_name(std::move(name)), _path(std::move(fpath)), _type(std::move(type)), _content(std::move(content))
{
}

FileSystem::INode::~INode()
{
}

void FileSystem::INode::buildDirectories(fs::path fpath, std::shared_ptr<INode> pa)
{
	/*
		创建的file的name不可以包含'/'
		创建的direcories的name不可以包含'.'和'/'
		使用是否包含'.'来判断文件类型
	*/
	int count = 0;

	for (auto& itr : fs::directory_iterator(fpath)) {
		auto title = itr.path();
		std::string str_title = title.generic_string();
		int pos = str_title.find_last_of('/') + 1;
		std::string fname = str_title.substr(pos);
		if (fname.find('.') == string::npos) {
			INode* tempchild = new INode(fname, fpath.generic_string(), kDirectory);
			std::shared_ptr<INode> child(tempchild);
			child->_parent = pa;
			child->buildDirectories(fpath / fname, child);
			_children.push_back(child);
		}
		else {
			std::ifstream infile;
			infile.open(fpath / fname);
			std::string content_line;
			std::string content = "";

			cout << fname << " " << count << endl;
			if (infile.is_open()) {
				while (getline(infile, content_line))
					content += content_line + "\n";		
			}
			else
				cout << "not open" << endl;
			//const string& name, const std::string& fpath, const Method& type, std::shared_ptr<INode> pa, const string& content = ""
			pa->addChild(fname, fpath.generic_string(), kCreateFile, pa, content);
		}

	}
}

bool FileSystem::INode::addChild(const string& name, const std::string& fpath, const Method& type, std::shared_ptr<INode> pa, const string& content) {
	ftype t = type == kCreateFile ? kFile : kDirectory;
	INode* rawchild = new INode(name, fpath, t, content);
	std::shared_ptr<INode> child(rawchild);
	rawchild->_parent = pa;
	_children.push_back(child);
	//build file in disk
	return true;
}

bool FileSystem::INode::eraseChild(const string& name) {
	for (_itr_node = _children.begin(); _itr_node != _children.end(); _itr_node++) {
		if ((*_itr_node)->_name == name)
			_children.erase(_itr_node);
	}
	return true;
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
		_root->buildDirectories(_absoluteRootPath, _workingDirectory);
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
	if(!_workingDirectory->ifChildreneExist())
		cout << "empty!" << endl;
	return _workingDirectory->getChildrenname();
}

std::future<bool> FileSystem::createFile(const std::string& name, const std::string& content)
{
	std::unique_lock<std::mutex> lck(_mutex);

	CreateFileParams param = { name, _workingDirectory->getPath() + "/" + name, content };
	IORequestPacket packet = { kCreateFile, param };
	packet.workingDirectory = std::shared_ptr(_workingDirectory);
	std::promise<bool> proObj;
	std::future<bool> fuObj = proObj.get_future();
	_messageQueue.emplace(packet, std::move(proObj));

	lck.unlock();
	_condition.notify_all();
	return fuObj;
}

std::future<bool> FileSystem::appendFile(const std::string& name, const std::string& content)
{
	std::unique_lock<std::mutex> lck(_mutex);
	
	AppendFileParams param = { name, content, _workingDirectory->getPath() + "/" + name };
	IORequestPacket packet = { kAppendFile, param };
	packet.workingDirectory = std::shared_ptr(_workingDirectory);
	std::promise<bool> proObj;
	std::future<bool> fuObj = proObj.get_future();
	_messageQueue.emplace(packet, std::move(proObj));

	lck.unlock();
	_condition.notify_all();
	return fuObj;
}

std::string FileSystem::loadFile(std::string name)
{
	_itr_node = _workingDirectory->getChildren();
	bool ifexist = false;
	for ( ; _itr_node != _workingDirectory->getChildren_end(); _itr_node++) {
		if ((*_itr_node)->getName() == name) 
			return (*_itr_node)->getContent();
	}
	return "no file named" + name;
}

std::future<bool> FileSystem::createDirectory(const std::string& name)
{
	std::unique_lock<std::mutex> lck(_mutex);

	CreateDirectoryParams param = { name, _workingDirectory->getPath() + "/" + name };
	IORequestPacket packet = { kMakeDirectory, param };
	packet.workingDirectory = std::shared_ptr(_workingDirectory);
	std::promise<bool> proObj;
	std::future<bool> fuObj = proObj.get_future();
	_messageQueue.emplace(packet, std::move(proObj));

	lck.unlock();
	_condition.notify_all();
	
	return fuObj;
}


std::future<bool> FileSystem::removeFile(const std::string& name)
{
	std::unique_lock<std::mutex> lck(_mutex);
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
		lck.unlock();
		_condition.notify_all();
		return result;
	}

	RemoveFileParams param = { name, _workingDirectory->getPath() };
	IORequestPacket packet = { kDeleteFile, param };
	packet.workingDirectory = std::shared_ptr(_workingDirectory);
	std::promise<bool> proObj;
	std::future<bool> fuObj = proObj.get_future();
	_messageQueue.emplace(packet, std::move(proObj));

	lck.unlock();
	_condition.notify_all();
	return fuObj;
}

std::future<bool> FileSystem::rename(const std::string& oldname, const std::string& newname)
{
	std::unique_lock<std::mutex> lck(_mutex);
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
		lck.unlock();
		_condition.notify_all();
		return result;
	}
	RenameParams param = { oldname, newname, _workingDirectory->getPath() };
	IORequestPacket packet = { kRename, param };
	packet.workingDirectory = std::shared_ptr(_workingDirectory);
	std::promise<bool> proObj;
	std::future<bool> fuObj = proObj.get_future();
	_messageQueue.emplace(packet, std::move(proObj));

	lck.unlock();
	_condition.notify_all();
	return fuObj;
}

std::future<bool> FileSystem::back()
{
	std::unique_lock<std::mutex> lck(_mutex);
	bool ifback;
	if (_workingDirectory != _root) {
		_workingDirectory = _workingDirectory->getParent();
		ifback = true;
	}
	else
		ifback = false;

	_workingDirectory->getChildrenname();
	cout << "working name end end" << endl;

	std::promise<bool> proObj;
	std::future<bool> fuObj = proObj.get_future();

	proObj.set_value(ifback);

	lck.unlock();
	_condition.notify_all();
	return fuObj;
}

std::future<bool> FileSystem::load(const string& name)
{
	std::unique_lock<std::mutex> lck(_mutex);

	bool ifexist = false;
	for (_itr_node = _workingDirectory->getChildren(); _itr_node != _workingDirectory->getChildren_end(); _itr_node++) {
		if ((*_itr_node)->getName() == name) {
			ifexist = true;
			_workingDirectory = std::shared_ptr(*_itr_node);
			break;
		}
	}
	std::promise<bool> proObj;
	std::future<bool> fuObj = proObj.get_future();
	proObj.set_value(ifexist);
	return fuObj;
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
		
			if (request.first.method == kCreateFile) {
				CreateFileParams param = std::get<CreateFileParams>(request.first.params);
				_itr_node = request.first.workingDirectory->getChildren();
				bool ifexist = false;
				for ( ; _itr_node != request.first.workingDirectory->getChildren_end(); _itr_node++) {
					if ((*_itr_node)->getName() == param.name) {
						request.second.set_value(false);
						ifexist = true;
						break;
					}
				}
				if (!ifexist) {
					request.first.workingDirectory->addChild(param.name, param.fpath, request.first.method, request.first.workingDirectory, param.content);
					request.second.set_value(true);

					std::ofstream outfile;
					fs::path filepath = request.first.workingDirectory->getPath() + '/' + request.first.workingDirectory->getName() + '/' + param.name;
					outfile.open(filepath, ios::out);
					if (outfile.is_open())
						outfile << param.content;
					outfile.close();
				}
			}
			else if (request.first.method == kDeleteFile) {
				RemoveFileParams param = std::get<RemoveFileParams>(request.first.params);
				_itr_node = request.first.workingDirectory->getChildren();
				bool ifdel = true;
				for (; _itr_node != request.first.workingDirectory->getChildren_end(); _itr_node++) {
					if ((*_itr_node)->getName() == param.name) {
						request.first.workingDirectory->eraseChild(param.name);
						request.second.set_value(true);
						ifdel = false;
						fs::remove_all(param.fpath);
						break;
					}
				}
				if (ifdel)
					request.second.set_value(false);
			}
			else if (request.first.method == kMakeDirectory) {
				CreateDirectoryParams param = std::get<CreateDirectoryParams>(request.first.params);
				bool ifexist = false;
				_workingDirectory->ifChildreneExist();

				if (request.first.workingDirectory->ifChildreneExist()) {
					_itr_node = request.first.workingDirectory->getChildren();
					for (; _itr_node != request.first.workingDirectory->getChildren_end(); _itr_node++) {
						if ((*_itr_node)->getName() == param.name) {
							request.second.set_value(false);
							ifexist = true;
							break;
						}
					}
				}
				if (!ifexist) {
					request.first.workingDirectory->addChild(param.name, param.fpath, request.first.method, request.first.workingDirectory);
					request.second.set_value(true);
					fs::create_directories(param.fpath);
				}
				
			}
			else if (request.first.method == kRename) {
				RenameParams param = std::get<RenameParams>(request.first.params);
				_itr_node = request.first.workingDirectory->getChildren();
				bool ifexist = false;
				bool ifrepeat = false;
				for ( ; _itr_node != request.first.workingDirectory->getChildren_end(); _itr_node++) {
					if ((*_itr_node)->getName() == param.oldname)
						ifexist = true;
					if ((*_itr_node)->getName() == param.newname)
						ifrepeat = true;
				}
				if (ifexist && !ifrepeat) {
					(*_itr_node)->setName(param.newname);
					fs::rename(fs::path(param.fpath) / param.oldname, fs::path(param.fpath) / param.newname);
					request.second.set_value(true);
				}
				else
					request.second.set_value(false);
			}
			else if (request.first.method == kAppendFile) {
				AppendFileParams param = std::get<AppendFileParams>(request.first.params);
				_itr_node = request.first.workingDirectory->getChildren();
				bool ifexist = false;
				for (; _itr_node != request.first.workingDirectory->getChildren_end(); _itr_node++) {
					if ((*_itr_node)->getName() == param.name && (*_itr_node)->iftxt()) {
						(*_itr_node)->append(param.content);

						// append file local
						std::fstream outfile;
						fs::path filepath = (*_itr_node)->getPath() + '/' + (*_itr_node)->getName();
						outfile.open(filepath, ios::app);
						if (outfile.is_open())
							outfile << param.content;
						outfile.close();

						ifexist = true;
						break;
					}
				}
				request.second.set_value(ifexist);
;			}
			_messageQueue.pop();
			lock.unlock();
			_condition.notify_all();
		}

		//process request


		//if(success)
		//request.second.set_value(true);
		//else
		//request.second.set_value(false);
	}
}
