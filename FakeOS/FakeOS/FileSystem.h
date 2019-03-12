#pragma once

#include "Kernel.h"
#include <string>
#include <array>
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
	void changeDirectory(const std::string& path);

	//functions below call _condition.notify() at the end

	//create a file at current directory
	[[nodiscard]] 
	std::future<bool> createFile(
		const std::string& name, 
		const std::string& path,  //like '/parent/path'
		const std::string& content = "");
	
	//create a directory at current directory
	[[nodiscard]]
	std::future<bool> createDirectory(
		const std::string& name,
		const std::string& path);  //like '/parent/path'

	//delete a file at current directory
	[[nodiscard]]
	std::future<bool> removeFile(
		const std::string& name,
		const std::string& path);  //like '/parent/path'
		
	//std::future<bool> rename();
	//std::future<bool> removeDirectory();
	//std::future<bool> copyFile();
	//std::future<bool> copyDirectory();

private:
	void threadFunc();

	class INode; //forward declaration
	
	enum Method: uint8_t
	{
		kCreateFile,
		kDeleteFile,
		kMakeDirectory
	};

	struct CreateFileParams
	{
		std::string name;
		std::string path;
		std::string content;
	};
	struct CreateDirectoryParams
	{
		std::string name;
		std::string path;
	};
	struct RemoveFileParams
	{
		std::string name;
		std::string path;
	};

	struct IORequestPacket
	{
		Method method;
		std::string workingDirectory;
		std::variant<
			CreateFileParams, 
			CreateDirectoryParams, 
			RemoveFileParams
		> params;
	};
	//int a = sizeof CreateFileParams;

	fs::path _absoluteRootPath;
	std::unique_ptr<INode> _root;
	std::unique_ptr<INode> _workingDirectory; //'cd' command changes this

	std::mutex _mutex;
	std::condition_variable _condition;
	//a produer-customer queue
	std::queue< std::pair<IORequestPacket, std::promise<bool>> > _messageQueue;

	std::atomic<bool> _quit;
	kernel::ThreadPtr _thread;
};
