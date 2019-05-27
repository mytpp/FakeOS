#include <string>

using namespace std;
//remove blanks from input string
string trim(string s)
{
	int index = 0;
	if (!s.empty())
	{
		while ((index = s.find(' ', index)) != string::npos)
		{
			s.erase(index, 1);
		}

	}
	return s;
}
void ExplainCmd(string cmd)
{
	cmd = trim(cmd);
	string str,name,content,address;
	FileSystem fs;
	int length;
	str = cmd.substr(0, 2);
	if (str == "cd")
	{
		address = cmd.substr(2);
		if (address == "..") fs.back();
	}
	else if(str=="cf")
	{
		name = cmd.substr(2, cmd.find("[")-2);
		content = cmd.substr(cmd.find("[") + 2, cmd.find("]")- cmd.find("[") - 2);
		fs.createFile(name, content);
	}
	else if (str == "af")
	{
		name = cmd.substr(2, cmd.find("\"") - 2);
		content = cmd.substr(cmd.find("\"") + 1, cmd.rfind("\"") - cmd.find("\"") - 1);
	}
	else if (str == "md")
	{
		name = cmd.substr(2);
		fs.createDirectory(name);
	}
	else if (str == "rm")
	{
		name = cmd.substr(2);
		fs.removeFile(name);
	}
	else if (str == "./")
	{

	}
}
