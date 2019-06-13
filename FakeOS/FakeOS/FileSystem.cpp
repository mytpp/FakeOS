#include "FileSystem.h"
#include <cassert>
#include <list>
#include <fstream>
#include <iostream>

/*待修改：
将判断io请求是否成功放入请求处理队列中
对锁定路径进行修改
文件读取*/

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
		if (_path == "")
			return _name;
		else
			return _path + '/' + _name;
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
		if (_name != "root")
			_parent.lock()->lockNodePath();
	}
	void unlockNodePath() {
		_lockCount -= 1;
		if (_name != "root")
			_parent.lock()->unlockNodePath();
	}
	void lockNode() {
		_lockCount += 1;
	}
	void unlockNode() {
		_lockCount -= 1;
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
		if ((*_itr_node)->_name == name) {
			_children.erase(_itr_node);
			break;
		}
	}
	return true;
}



FileSystem::FileSystem()
	:_quit(false)
	, _thread(nullptr, kernel::ThreadDeleter)
{
	ProcessNum = 0;
	if (!fs::exists("root")) {
		fs::create_directory("root");
		_absoluteRootPath = fs::path("root");
		_root = std::shared_ptr<INode>(new INode("root", "", kDirectory));
		_workingDirectory = _root;
	}
	else {
		_absoluteRootPath = fs::path("root");
		_root = std::shared_ptr<INode>(new INode("root", "", kDirectory));
		_workingDirectory = _root;
		_root->buildDirectories(_absoluteRootPath, _workingDirectory);
	}
	_workingDirectory->lockNode();
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



std::vector<std::string> FileSystem::list(const int& pno)
{
	if (pno == 0) {
		if (!_workingDirectory->ifChildreneExist())
			cout << "empty!" << endl;
		return _workingDirectory->getChildrenname();
	}
	else {
		for (_itr_fp = _processFptrList.begin(); _itr_fp != _processFptrList.end(); _itr_fp++) {
			if ((*_itr_fp).No == pno)
				return (*_itr_fp).processWorkingDirct->getChildrenname();
		}
	}
}

std::future<bool> FileSystem::appendFile(const std::string& name, const std::string& content, const int& pno)
{
	std::unique_lock<std::mutex> lck(_mutex);

	AppendFileParams param = { name, content };//, _workingDirectory->getPath() };
	IORequestPacket packet = { kAppendFile };//, param };
	if (pno == 0) {
		packet.workingDirectory = std::shared_ptr(_workingDirectory);
		param.fpath = _workingDirectory->getPath();
		packet.params = param;

	}
	else {
		for (_itr_fp = _processFptrList.begin(); _itr_fp != _processFptrList.end(); _itr_fp++) {
			if ((*_itr_fp).No == pno) {
				packet.workingDirectory = std::shared_ptr((*_itr_fp).processWorkingDirct);
				param.fpath = (*_itr_fp).processWorkingDirct->getPath();
				packet.params = param;
			}
		}
	}

	std::promise<bool> proObj;
	std::future<bool> fuObj = proObj.get_future();
	_messageQueue.emplace(packet, std::move(proObj));

	lck.unlock();
	_condition.notify_all();
	return fuObj;
}

std::future<bool> FileSystem::createFile(const std::string& name, const std::string& content, const int& pno)
{
	std::unique_lock<std::mutex> lck(_mutex);
	CreateFileParams param = { name, content };
	IORequestPacket packet = { kCreateFile };//, param };

	if (pno == 0) {
		packet.workingDirectory = std::shared_ptr(_workingDirectory);
		param.fpath = _workingDirectory->getPath();
		packet.params = param;
		cout << "..............." << endl;
		cout << std::get<CreateFileParams>(packet.params).content << endl;
	}
	else {
		for (_itr_fp = _processFptrList.begin(); _itr_fp != _processFptrList.end(); _itr_fp++) {
			if ((*_itr_fp).No == pno) {
				packet.workingDirectory = std::shared_ptr((*_itr_fp).processWorkingDirct);
				param.fpath = (*_itr_fp).processWorkingDirct->getPath();
				packet.params = param;
			}
		}
	}

	std::promise<bool> proObj;
	std::future<bool> fuObj = proObj.get_future();
	_messageQueue.emplace(packet, std::move(proObj));

	lck.unlock();
	_condition.notify_all();
	return fuObj;
}


std::string FileSystem::loadFile(std::string name, const int& pno)
{
	bool ifexist = false;
	if (pno == 0) {
		_itr_node = _workingDirectory->getChildren();
		for (; _itr_node != _workingDirectory->getChildren_end(); _itr_node++) {
			if ((*_itr_node)->getName() == name) {
				(*_itr_node)->lockNode();
				return (*_itr_node)->getContent();
			}
		}
	}
	else {
		
		for (_itr_fp = _processFptrList.begin(); _itr_fp != _processFptrList.end(); _itr_fp++) {
			if ((*_itr_fp).No == pno) {
				_itr_node = (*_itr_fp).processWorkingDirct->getChildren();
				for (; _itr_node != (*_itr_fp).processWorkingDirct->getChildren_end(); _itr_node++) {
					if ((*_itr_node)->getName() == name) {
						(*_itr_node)->lockNode();
						return (*_itr_node)->getContent();
					}
				}
			}
		}
	}
	return "no file";
}

std::future<bool> FileSystem::createDirectory(const std::string& name,const int& pno)
{
	std::unique_lock<std::mutex> lck(_mutex);
	CreateDirectoryParams param = { name };//, _workingDirectory->getPath() };
	IORequestPacket packet = { kMakeDirectory };//, param };
	if (pno == 0) {
		packet.workingDirectory = std::shared_ptr(_workingDirectory);
		param.fpath = _workingDirectory->getPath();
		packet.params = param;

	}
	else {
		for (_itr_fp = _processFptrList.begin(); _itr_fp != _processFptrList.end(); _itr_fp++) {
			if ((*_itr_fp).No == pno) {
				packet.workingDirectory = std::shared_ptr((*_itr_fp).processWorkingDirct);
				param.fpath = (*_itr_fp).processWorkingDirct->getPath();
				packet.params = param;
			}
		}
	}
	std::promise<bool> proObj;
	std::future<bool> fuObj = proObj.get_future();
	_messageQueue.emplace(packet, std::move(proObj));

	lck.unlock();
	_condition.notify_all();
	
	return fuObj;
}

// nodicard
std::future<bool> FileSystem::removeFile(const std::string& name, const int& pno)
{
	std::unique_lock<std::mutex> lck(_mutex);
	bool ifexist = false;

	RemoveFileParams param = { name };//, _workingDirectory->getPath() };
	IORequestPacket packet = { kDeleteFile };//, param };
	if (pno == 0) {
		packet.workingDirectory = std::shared_ptr(_workingDirectory);
		param.fpath = _workingDirectory->getPath();
		packet.params = param;

	}
	else {
		for (_itr_fp = _processFptrList.begin(); _itr_fp != _processFptrList.end(); _itr_fp++) {
			if ((*_itr_fp).No == pno) {
				packet.workingDirectory = std::shared_ptr((*_itr_fp).processWorkingDirct);
				param.fpath = (*_itr_fp).processWorkingDirct->getPath();
				packet.params = param;
			}
		}
	}
	std::promise<bool> proObj;
	std::future<bool> fuObj = proObj.get_future();
	_messageQueue.emplace(packet, std::move(proObj));

	lck.unlock();
	_condition.notify_all();
	return fuObj;
}

std::future<bool> FileSystem::back(const int& pno)
{
	std::unique_lock<std::mutex> lck(_mutex);
	bool ifback;
	if (pno == 0) {
		if (_workingDirectory != _root) {
			_workingDirectory->unlockNode();
			_workingDirectory = _workingDirectory->getParent();
			ifback = true;
		}
		else
			ifback = false;
	}
	else {
		for (_itr_fp = _processFptrList.begin(); _itr_fp != _processFptrList.end(); _itr_fp++) {
			if ((*_itr_fp).No == pno) {
				if ((*_itr_fp).processWorkingDirct != _root) {
					(*_itr_fp).processWorkingDirct->unlockNode();
					(*_itr_fp).processWorkingDirct = (*_itr_fp).processWorkingDirct->getParent();
					ifback = true;
				}
				else
					ifback = false;
			}
		}
	}

	std::promise<bool> proObj;
	std::future<bool> fuObj = proObj.get_future();

	proObj.set_value(ifback);

	lck.unlock();
	_condition.notify_all();
	return fuObj;
}

std::future<bool> FileSystem::load(const string& name, const int& pno)
{
	std::unique_lock<std::mutex> lck(_mutex);

	bool ifexist = false;
	if (pno == 0) {
		for (_itr_node = _workingDirectory->getChildren(); _itr_node != _workingDirectory->getChildren_end(); _itr_node++) {
			if ((*_itr_node)->getName() == name) {
				ifexist = true;
				_workingDirectory = std::shared_ptr(*_itr_node);
				(*_itr_node)->lockNodePath();
				break;
			}
		}
	}
	else {
		for (_itr_fp = _processFptrList.begin(); _itr_fp != _processFptrList.end(); _itr_fp++) {
			if ((*_itr_fp).No == pno) {
				for (_itr_node = (*_itr_fp).processWorkingDirct->getChildren(); _itr_node != (*_itr_fp).processWorkingDirct->getChildren_end(); _itr_node++) {
					if ((*_itr_node)->getName() == name) {
						ifexist = true;
						(*_itr_fp).processWorkingDirct = std::shared_ptr(*_itr_node);
						(*_itr_node)->lockNodePath();
						break;
					}
				}
			}
		}
	}

	std::promise<bool> proObj;
	std::future<bool> fuObj = proObj.get_future();
	proObj.set_value(ifexist);
	return fuObj;
}

std::string FileSystem::nowpath(const int& pno)
{
	if (pno == 0)
		return _workingDirectory->getPath();
	else {
		for (_itr_fp = _processFptrList.begin(); _itr_fp != _processFptrList.end(); _itr_fp++) {
			if ((*_itr_fp).No == pno) {
				return (*_itr_fp).processWorkingDirct->getPath();
			}
		}
	}
}

int FileSystem::allocateFptr(const int& curNo)
{
	ProcessNum++;
	filePtr newfp = { ProcessNum };
	//printf("33333");
	if (curNo == 0) 	
		newfp.processWorkingDirct = std::shared_ptr(_workingDirectory);
	else
	{
		for (_itr_fp = _processFptrList.begin(); _itr_fp != _processFptrList.end(); _itr_fp++) {
			if ((*_itr_fp).No == curNo) 
				newfp.processWorkingDirct = std::shared_ptr((*_itr_fp).processWorkingDirct);
		}
	}
	newfp.processWorkingDirct->lockNodePath();
	_processFptrList.push_back(newfp);
	return ProcessNum;
}

void FileSystem::killFptr(const int& curNo)
{
	for (_itr_fp = _processFptrList.begin(); _itr_fp != _processFptrList.end(); _itr_fp++) {
		if ((*_itr_fp).No == curNo)
			(*_itr_fp).processWorkingDirct->unlockNodePath();
	}
	
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
				cout << "-----------";
				cout << param.content << endl;
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

					std::fstream outfile;
					outfile.open(param.fpath + '/' + param.name, ios::out);
					if (outfile.is_open())
						outfile << param.content;
					else
						cout << "open fail!" << endl;
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
						fs::remove_all(param.fpath + '/' + param.name);
						request.second.set_value(true);
						ifdel = false;
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
					fs::create_directories(param.fpath + '/' + param.name);
				}
				
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
						fs::path filepath = param.fpath + '/' + param.name;
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
