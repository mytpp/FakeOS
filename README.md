# FakeOS
Course work for OS

* C++17
* x86

***
command line support:  
* cd /absolute/path (relative to the default path that is set for FakeOS.exe )
* cd relative/relative (relative to current working directory)
* cd ..
* cf FileName ["initial content"]
* af FileName "appended content"
* md NewDirectoryName
* rm FileName
* rm DirectoryName
* ./ProcessScript [priority]

***
A mocked process is a text file like a kind of script language. A sample process 'script' is as follows:
```
# priority (optional)
5 DemandMemory 12345 As addr1
2 CreateFile newfilename some initial content
3 DeleteFile existingfilename
```
Explanation:   
The first line indicate the priority of the process. Default priority is 'Normal'.
Each line below represents a directive. Each lexical symbol is seperated by blankspace.
In a single line, the first symbol (a number) represent the time (how many CPU cycle) 
elapsed since the last directive executed. The second symbol represents what kind of 
directive will be executed. The extra symbols represents parameters that the directive needs. 
The interpreter will discard unrecognizable directives.    
Now totally 5 directives are supported:   

Directive                     | var1        | var2
------------------------------|-------------|-------------------------
DemandMemory (var1) As (var2) | size        | number of the start page (this value will be stored in PCB), and can be used by FreeMemory and AccessMemory
FreeMemory (var1)             | start page  |
AccessMemory (var1)           | page number |
CreateFile (var1) (var2)      | filename    | initial content
DeleteFile (var1)             | filename    |
