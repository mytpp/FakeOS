#pragma once

#include "Kernel.h"
#include <string>
#include <array>
#include <vector>
#include <queue>
#include <atomic>
#include <future>
#include <condition_variable>
#include <variant>
#include <filesystem>


namespace fs = std::filesystem;

class FileSystem
{
public:
	FileSystem();
	~FileSystem();

	void start();
	void quit();
	//Lock current path
	void lockPath();
	//Unlock a path. It's called when a mocked process died.
	//Its' parameter is passed from PCB
	void unLockPath(std::array<uint8_t, 8> path);


	//this function doesn't need to pack IORequestPacket
	//because 'where is current path' is stored only in memory
	//bool changeDirectory(const std::string& path);	

	//ls
	std::vector<std::string> list(const int& pno=0);

	//functions below call _condition.notify() at the end

	//create a file at current directory
	std::future<bool> createFile(
		const std::string& name, 
		const std::string& content,
		const int& pno=0);

	//append some content to a file
	[[nodiscard]]
	std::future<bool> appendFile(
		const std::string& name,
		const std::string& content,
		const int& pno=0);

	//open a .txt file in command line
	std::string loadFile(
		std::string name,
		const int& pno=0);
	//create a directory at current directory
	[[nodiscard]]
	std::future<bool> createDirectory(
		const std::string& name,
		const int& pno=0);

	//delete a file at current directory
	[[nodiscard]]
	std::future<bool> removeFile(
		const std::string& name,
		const int& pno=0);
		
	//std::future<bool> rename(const std::string& oldname, const std::string& newname);
	//std::future<bool> copyFile();
	//std::future<bool> copyDirectory();

	//cd ..
	std::future<bool> back(const int& pno=0);
	//cd
	std::future<bool> load(
		const std::string &name,
		const int& pno=0);
	//
	std::string nowpath(const int& pno=0);

	// allocate file pointer for process
	int allocateFptr(const int& curNo=0);
	void killFptr(const int& curNo);

private:

	void threadFunc();

	class INode; //forward declaration
	
	enum Method: uint8_t
	{
		kCreateFile	,
		kDeleteFile,
		kMakeDirectory,
		kRename,
		kAppendFile
	};
	struct filePtr {
		int No;
		std::shared_ptr<INode> processWorkingDirct;
	};
	struct CreateFileParams
	{
		std::string name;
		std::string content;
		std::string fpath;
	};
	struct CreateDirectoryParams
	{
		std::string name;
		std::string fpath;
	};
	struct RemoveFileParams
	{
		std::string name;
		std::string fpath;
	};
	struct RenameParams
	{
		std::string oldname;
		std::string newname;
		std::string fpath;
	};
	struct AppendFileParams
	{
		std::string name;
		std::string content;
		std::string fpath;
	};

	struct IORequestPacket	
	{
		Method method;	//type of request 
		std::variant <
			CreateFileParams,
			CreateDirectoryParams,
			RemoveFileParams,
			RenameParams,
			AppendFileParams
		> params;
		std::shared_ptr<INode> workingDirectory;	//	parent path
	};
	

	fs::path _absoluteRootPath;
	std::shared_ptr<INode> _root;
	std::shared_ptr<INode> _workingDirectory; //'cd' and 'cd..' command changes this

	std::mutex _mutex;
	std::condition_variable _condition;
	//a produer-customer queue
	std::queue< std::pair<IORequestPacket, std::promise<bool>> > _messageQueue;

	std::atomic<bool> _quit;
	kernel::ThreadPtr _thread;
	std::list<std::shared_ptr<INode>>::iterator _itr_node;
	int ProcessNum;
	std::list<filePtr> _processFptrList;
	std::list<filePtr>::iterator _itr_fp;
};

