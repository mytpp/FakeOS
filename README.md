# FakeOS
Course work for OS

* C++17
* x86

***

A mocked process is a text file like a kind of script language. A sample process 'script' is as follows:
```
5 DemandMemory 12345
2 CreateFile newfilename some initial content
3 DeleteFile existingfilename
```
explanation:   
Each line represents a directive. Each lexical symbol is seperated by blankspace.
In a single line, the first symbol (a number) represent the time (how many CPU cycle) 
elapsed since the last directive executed. The second symbol represents what kind of 
directive will be executed. The extra symbols represents parameters that the directive needs. 
The interpreter will discard unrecognizable directive.    
Now totally 3 commands are supported:   

Command      | params
-------------|--------
DemandMemory | size
CreateFile   | filename + initial content
DeleteFile   | filename
