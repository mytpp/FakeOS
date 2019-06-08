#include "ScheduleQueue.h"
#include "MemoryManager.h"
#include "ProcessScheduler.h"
#include "FileSystem.h"
using namespace std;
//unique_ptr<ProcessScheduler> processScheduler;
//unique_ptr<FileSystem> fileSystem;

using namespace kernel;

namespace ScheduleQueue
{
	/*

	template<typename rtnTtpe, typename ArgType>
	class object {

	};
	
	
	
	enum Interrupt
	{
		IO,
		End,
		No
	};
	enum ProcessState
	{
		Block,
		Ready,
		Execute,
		White
	};
	enum DeviceType
	{
		a,
		b,
		c,
		no
	};
	class DeviceStateChangeEventArgs
	{
	private:int _Atime;
			DeviceType _type;
			int _DN;
			string _processname;
			int _needtime;
	public:int Atime_get() { return _Atime; }
		   void	 Atime_set(int value) { _Atime = value; }
		   DeviceType type_get() { return _type; }
		   void	 type_set(DeviceType value) { _type = value; }
		   int DN_get() { return _DN; }
		   void DN_set(int value) { _DN = value; }
		   string processname_get() { return _processname; }
		   void   processname_set(string value) { _processname = value; }
		   int needtime_get() { return _needtime; }
		   void needtime_set(int value) { _needtime = value; }
	};
	DeviceStateChangeEventArgs EventArgs;
	struct PCB
	{
		int ProcessID;                           //进程块的编号（0-9）
		string ProcessName;                      //使用该进程块的进程名
		int PageAdress;                          //页表的首地址
		int Sum;                                 //页表的长度    
		int PC;                                  //各个寄存器的状态
		string IR;
		int DR;
		Interrupt PSW;
		int Pri;                                 //优先级
		int WaitTime;                            //要使用设备多长时间
		int GetDeviceTime;                       //获得设备的时间
		int ExecuteTime;                         //开始执行的时间
		DeviceType NeedDevice;                   //申请失败的设备类型
		DeviceType HaveDevice;                   //正在使用的设备类型
		int DN;                                  //使用的是哪个设备
		string BlockReason;                      //阻塞的原因
		int Next;
	};
	class CPU
	{
		class Device
		{
			class DeviceTable
			{
			public:
				DeviceType deviceType;
				int total;
				int* useState;
				DeviceTable(DeviceType type, int total)
				{
					total = total;
					deviceType = type;
					useState = new int[total];
					for (int i = 0; i < total; i++)
					{
						useState[i] = 0;
					}
				}
				DeviceTable() {

				}
			};
		private:
			DeviceTable* table = new DeviceTable[3];
			void Init()
			{
				*table = DeviceTable(a, 3);
				*(table+1) = DeviceTable(b, 2);
				*(table+2) = DeviceTable(c, 1);
			};
		public:
			Device() {
				Init();
			}
			//
			//检查类型为type的设备是否可用
			//
			bool JudgeDevice(DeviceType type) {
				bool str = false;
				switch (type)
				{
				case a:
				{
					if (table[0].total > 0)
					{
						str = true;
					}
					break;
				}
				case b:
				{
					if (table[1].total > 0)
					{
						str = true;
					}
					break;
				}
				case c:
				{
					if (table[2].total > 0)
					{
						str = true;
					}
					break;
				}
				}
				return str;
			}
			//
			//分配设备,返回第几个设备被占用
			//
			int Allocate(DeviceType type) {
				int k = 0;
				switch (type)
				{
				case a:
				{
					table[0].total--;
					for (int i = 0; i < 3; i++)
					{
						if (table[0].useState[i] == 0)
						{
							table[0].useState[i] = 1;
							k = i;
							break;
						}
					}
					break;
				}
				case b:
				{
					table[1].total--;
					for (int i = 0; i < 2; i++)
					{
						if (table[0].useState[i] == 0)
						{
							table[0].useState[i] = 1;
							k = i;
							break;
						}
					}
					break;
				}
				case c:
				{
					table[2].total--;
					break;
				}
				}
				return k;
			}
			//
			//回收设备
			//
			void DeAllocate(DeviceType type, int aa)
			{
				switch (type)
				{
				case a:
				{
					table[0].total++;
					table[0].useState[aa] = 0;
					break;
				}
				case b:
				{
					table[1].total++;
					table[1].useState[aa] = 0;
					break;
				}
				case c:
				{
					table[2].total++;
					table[2].useState[aa] = 0;
					break;
				}
				}
			}

		};
		public: int PC;
				int DR;
				string IR;
			    Interrupt PSW;
				Interrupt PSW1;
				Device* Dev = new Device();
				PCB* PCBArray = new PCB[10];
				DeviceStateChangeEventArgs* DeviceStateChange;
				//DateTime XTTime;
				int XDTime;
				int White;
				int Ready;
				int Block;
				int Execute;
				DeviceType type;
				int time;
				//OS.ClassFolder.MainRam ram = new MainRam();
				//OS.ClassFolder.Device Dev = new Device();
				CPU()
				{
					Init();
				};
				int GetOneFromWhite()
				{
					int a = White;
					if (a < 10)
					{
						White = PCBArray[a].Next;
					}
					return a;
				}
				void InsertOneToWhite(int a)
				{
					PCBArray[a].Next = White;
					White = a;
				}
				//
				//就绪PCB链表的操作
				//
				void InsertOneToReady(int a)
				{
					PCBArray[a].Next = Ready;
					Ready = a;
				}
				void GetOneFromReady(int a)
				{
					int b = Ready;
					if (a == b)
					{
						Ready = PCBArray[a].Next;
					}
					else
					{
						while (b < 10)
						{
							if (PCBArray[b].Next == a)
							{
								PCBArray[b].Next = PCBArray[PCBArray[b].Next].Next;
							}
							b = PCBArray[b].Next;
						}
					}
				}
				//
				//阻塞PCB链表的操作
				//
				void InsertOneToBlock(int a)
				{
					PCBArray[a].Next = Block;
					Block = a;
				}
				//
				//Creat函数，创建进程
				//
				void Creat(string Name, std::string str)
				{
					//
					//申请PCB，a>10，则申请失败
					//
					int a = GetOneFromWhite();
					int b;
					if (str.size() > 0)
					{
						int sum = (str.length() + 15) / 16;
						if (a < 10)
						{
							if (ram.Judge(sum) == true) //注意下面要加载到内存
							{
								//
								//分配内存并加载到内存
								//
								//b = ram.Allocate(sum);
								//ram.LoadContent(str, b);
								//
								//初始化PCB
								//
								PCBArray[a].ProcessName = Name;
								PCBArray[a].PageAdress = b;
								PCBArray[a].Sum = sum;
								PCBArray[a].PC = 0;
								PCBArray[a].IR = "";
								PCBArray[a].DR = 0;
								PCBArray[a].PSW = No;
								PCBArray[a].WaitTime = -10;
								PCBArray[a].Pri = 1024 / str.length();
								PCBArray[a].ExecuteTime = 0;
								PCBArray[a].NeedDevice = no;
								PCBArray[a].HaveDevice = no;
								PCBArray[a].GetDeviceTime = 0;
								PCBArray[a].DN = -1;
								PCBArray[a].BlockReason = "";
								InsertOneToReady(a);
								//
								//是否转向进程调度
								//
								int c = JudgeAttemper();
								if (c < 10)
								{
									Attemper(c);
								}

							}
							else
							{
								InsertOneToWhite(a);
								//内存不足或文件太长，创建进程失败
							}
						}
						else
						{
							//PCB块不足，创建进程失败
						}
					}
					else
					{
						//文件为空,不能创建进程
					}
				};
				//
				//Destroy函数，撤销程序
				//
				void Destory(int a)
				{
					//
					//回收内存
					//
					int p = PCBArray[a].PageAdress;
					int sum = PCBArray[a].Sum;
					ram.DeAllocate(p, sum);
					//
					//回收PCB块
					//
					InsertOneToWhite(a);
					Execute = 10;
					//
					//显示结果
					//
					//
					//
					//


				};
				//
				//BlockProcess函数，阻塞进程
				//
				void BlockProcess(int a, DeviceType b, int time)
				{
					//
					//保护CPU现场
					//
					PCBArray[a].PC = PC;
					PCBArray[a].IR = IR;
					PCBArray[a].DR = DR;
					//
					//判断申请设备是否成功，根据不同情况填写BlockReason项
					//
					bool d = Dev->JudgeDevice(b);
					if (d == false)
					{
						PCBArray[a].NeedDevice = b;
						PCBArray[a].HaveDevice = no;
						PCBArray[a].BlockReason = "申请" + b;
						PCBArray[a].BlockReason = PCBArray[a].BlockReason + "设备失败";
						PCBArray[a].PC = PCBArray[a].PC - 4;
					}
					else
					{
						PCBArray[a].DN = Dev->Allocate(b);
						PCBArray[a].HaveDevice = b;
						PCBArray[a].NeedDevice = no;
						PCBArray[a].GetDeviceTime = XDTime;
						PCBArray[a].WaitTime = time;
						PCBArray[a].BlockReason = "等待IO输入输出";
						if (DeviceStateChange != NULL)
						{
							DeviceStateChangeEventArgs* e = new DeviceStateChangeEventArgs();
							e->DN_set(PCBArray[a].DN);
							e->type_set(b);
							e->processname_set(PCBArray[a].ProcessName);
							e->needtime_set(time);
							e->Atime_set(XDTime);
							DeviceStateChange = e;
						}
					}
					//
					//修改进程状态
					//
					InsertOneToBlock(a);
					Execute = 10;
					//
					//转向进程调度
					//
					int c = JudgeAttemper();
					if (c < 10)
					{
						Attemper(c);
					}

				};
				//
				//WakeUp函数，唤醒进程
				//
				void WakeUp(int ID, DeviceType device)
				{
					//
					//唤醒自己
					//
					int d = Block;
					if (Block == ID)
					{
						Block = PCBArray[ID].Next;
						InsertOneToReady(ID);
					}
					else
					{
						while (PCBArray[d].Next < 10)
						{
							if (PCBArray[d].Next == ID)
							{
								PCBArray[d].Next = PCBArray[ID].Next;
								InsertOneToReady(ID);
								break;
							}
							d = PCBArray[d].Next;
						}
					}
					//
					//检查第一个节点
					//
					while (Block < 10 && PCBArray[Block].NeedDevice == device)
					{
						int h = Block;
						Block = PCBArray[h].Next;
						InsertOneToReady(h);
					}
					//
					//检查其他节点
					//
					if (Block < 10)
					{
						int a = Block;
						while (PCBArray[a].Next < 10)
						{
							if (PCBArray[PCBArray[a].Next].NeedDevice == device)
							{
								int h = PCBArray[a].Next;
								PCBArray[a].Next = PCBArray[PCBArray[a].Next].Next;
								InsertOneToReady(h);
							}
							else
							{
								a = PCBArray[a].Next;
							}
						}
					}
					int c = JudgeAttemper();
					if (c < 10)
					{
						Attemper(c);
					}
				};
				//
				//判断是否需要进行进程的调度，若需要则返回进程块号（0-9），不需要则返回10
				//
				int JudgeAttemper()
				{
					//
					//选出就绪链表中优先级最高
					//
					int k;
					if (Ready < 10)
					{
						int p = Ready;
						int a = PCBArray[Ready].Pri;
						k = Ready;                                      //优先级最高的块号
						while (PCBArray[p].Next < 10)
						{
							if (PCBArray[PCBArray[p].Next].Pri > a)
							{
								a = PCBArray[PCBArray[p].Next].Pri;
								k = PCBArray[p].Next;
							}
							p = PCBArray[p].Next;
						}
					}
					else
					{
						return 10;
					}
					//
					//跟执行链表内的PCB块进行比较
					//
					if (Execute < 10)
					{
						if (PCBArray[k].Pri > PCBArray[Execute].Pri)
						{
							return k;
						}
						else
						{
							return 10;
						}
					}
					else
					{
						return k;
					}
				}
				//
				//进程调度函数
				//
				void Attemper(int a)
				{
					//
					//保护CPU现场
					//
					if (Execute < 10)
					{
						PCBArray[Execute].PC = PC;
						PCBArray[Execute].IR = IR;
						PCBArray[Execute].DR = DR;
						InsertOneToReady(Execute);
					}
					//
					//选择一个进程，初始化CPU中的寄存器
					//

					GetOneFromReady(a);
					Execute = a;
					PC = PCBArray[a].PC;
					IR = PCBArray[a].IR;
					DR = PCBArray[a].DR;

				};
				//
				//主函数cpu
				//
				void cpu()
				{

					
				};
		private:
			void Init()
			{
				//
				//初始化PCB块
				//
				White = 0;
				Ready = Block = Execute = 10;
				for (int i = 0; i < 10; i++)
				{
					PCBArray[i].ProcessID = i;
					PCBArray[i].Next = i + 1;
				}
				//
				//初始化寄存器
				//
				PC = 0;
				PSW = No;
				PSW1 = No;
				IR = "";
				//
				//初始化时间
				//
				//XTTime = Convert.ToDateTime("00:00:00");
				XDTime = 0;
			}
			//
			//构造函数
			//
				//
				//申请设备的事件和委托
				//
			
			//
			//空闲PCB链表的操作
			//
	};*/
		
		
	mutex newlyCreatedQueueMutex;
	mutex readyQueueMutex;
	mutex waitingQueueMutex;


	void LoadProcess(const std::string& path, uint16_t file_ptr)
	{
		//create PCB
		//put PCB in newlyCreatedQueueMutex
		static uint16_t time_count = 0;
		static uint16_t nextPid = 0;
		shared_ptr<PCB> pcb = make_shared<PCB>();
		pcb->pid = nextPid++;
		pcb->pageTable = make_unique<PageTable>();
		for (auto& pte : *(pcb->pageTable)) {
			pte.free = true;
		}
		pcb->file_ptr = fileSystem->allocateFptr(file_ptr);
		string tmp_str = fileSystem->loadFile(path, pcb->file_ptr);
		uint16_t index = tmp_str.find('\n');
		/*initialize the done code-position*/
		string priority_line = tmp_str.substr(0,index);
		pcb->priority = (ScheduleQueue::Priority)(priority_line.at(priority_line.length()-1) - '0');
		pcb->restCode = tmp_str.substr(index+1);
		/*ProgramCounter should be set as the first damand's Counter*/
		/*Predicted Counter could be all the counters' sum*/
		pcb->predictedCount = 0;
		string tmpStr = pcb->restCode.substr(0, pcb->restCode.length() - 1);

		for (; tmpStr.length() != 0 && tmpStr.find('\n') != tmpStr.npos; ) {
			pcb->predictedCount += (uint16_t)(tmpStr.at(0) - '0');
			tmpStr = tmpStr.substr(tmp_str.find('\n') + 1, tmpStr.length() - 1);
		}
		if (tmpStr.length() > 0) {
			pcb->predictedCount += (uint16_t)(tmpStr.at(0) - '0');
		}
		pcb->statistics.timeCreated = time_count++;
		pcb->statistics.usedCPUTime = 0;
		pcb->path = path;
		pcb->state = kNew;
		pcb->programCounter = pcb->restCode.at(0);
		std::unique_lock<std::mutex> lck(newlyCreatedQueueMutex);
		processScheduler->CreateProcess(pcb);
		lck.unlock();
	}
}
