#pragma once

#include "Kernel.h"
#include <string>
#include <queue>
#include <atomic>
#include <future>
#include <condition_variable>

class FileSystem
{
public:
	enum Type : uint8_t
	{
		kText,
		kExecutable
	};

	FileSystem();
	~FileSystem();

	void start();
	void quit();


	//this function doesn't need to pack IORequestPacket
	//because 'where is current path' is stored only in memory
	void changeDirectory(const std::string& path);


	//create a file at current directory
	[[nodiscard]] 
	std::future<bool> createFile(const std::string& name, 
								const std::string& content = "", 
								Type type = kText);
	

	//delete a file at current directory
	[[nodiscard]]
	std::future<bool> deleteFile(const std::string& name);


private:
	class INode; //forward declaration
	
	enum Method: uint8_t
	{
		kCreateFile,
		kDeleteFile,
		kMakeDirectory
	};

	struct IORequestPacket
	{
		Method method;
		std::unique_ptr<INode> workingDirectory;
	};

	void threadFunc();

	std::unique_ptr<INode> _root;
	std::unique_ptr<INode> _workingDirectory; //'cd' command changes this

	std::mutex _mutex;
	std::condition_variable _condition;
	//a produer-customer queue
	std::queue< std::pair<IORequestPacket, std::promise<bool>> > _messageQueue;

	std::atomic<bool> _quit;
	kernel::ThreadPtr _thread;
};

