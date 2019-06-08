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
		int ProcessID;                           //���̿�ı�ţ�0-9��
		string ProcessName;                      //ʹ�øý��̿�Ľ�����
		int PageAdress;                          //ҳ����׵�ַ
		int Sum;                                 //ҳ��ĳ���    
		int PC;                                  //�����Ĵ�����״̬
		string IR;
		int DR;
		Interrupt PSW;
		int Pri;                                 //���ȼ�
		int WaitTime;                            //Ҫʹ���豸�೤ʱ��
		int GetDeviceTime;                       //����豸��ʱ��
		int ExecuteTime;                         //��ʼִ�е�ʱ��
		DeviceType NeedDevice;                   //����ʧ�ܵ��豸����
		DeviceType HaveDevice;                   //����ʹ�õ��豸����
		int DN;                                  //ʹ�õ����ĸ��豸
		string BlockReason;                      //������ԭ��
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
			//�������Ϊtype���豸�Ƿ����
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
			//�����豸,���صڼ����豸��ռ��
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
			//�����豸
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
				//����PCB����Ĳ���
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
				//����PCB����Ĳ���
				//
				void InsertOneToBlock(int a)
				{
					PCBArray[a].Next = Block;
					Block = a;
				}
				//
				//Creat��������������
				//
				void Creat(string Name, std::string str)
				{
					//
					//����PCB��a>10��������ʧ��
					//
					int a = GetOneFromWhite();
					int b;
					if (str.size() > 0)
					{
						int sum = (str.length() + 15) / 16;
						if (a < 10)
						{
							if (ram.Judge(sum) == true) //ע������Ҫ���ص��ڴ�
							{
								//
								//�����ڴ沢���ص��ڴ�
								//
								//b = ram.Allocate(sum);
								//ram.LoadContent(str, b);
								//
								//��ʼ��PCB
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
								//�Ƿ�ת����̵���
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
								//�ڴ治����ļ�̫������������ʧ��
							}
						}
						else
						{
							//PCB�鲻�㣬��������ʧ��
						}
					}
					else
					{
						//�ļ�Ϊ��,���ܴ�������
					}
				};
				//
				//Destroy��������������
				//
				void Destory(int a)
				{
					//
					//�����ڴ�
					//
					int p = PCBArray[a].PageAdress;
					int sum = PCBArray[a].Sum;
					ram.DeAllocate(p, sum);
					//
					//����PCB��
					//
					InsertOneToWhite(a);
					Execute = 10;
					//
					//��ʾ���
					//
					//
					//
					//


				};
				//
				//BlockProcess��������������
				//
				void BlockProcess(int a, DeviceType b, int time)
				{
					//
					//����CPU�ֳ�
					//
					PCBArray[a].PC = PC;
					PCBArray[a].IR = IR;
					PCBArray[a].DR = DR;
					//
					//�ж������豸�Ƿ�ɹ������ݲ�ͬ�����дBlockReason��
					//
					bool d = Dev->JudgeDevice(b);
					if (d == false)
					{
						PCBArray[a].NeedDevice = b;
						PCBArray[a].HaveDevice = no;
						PCBArray[a].BlockReason = "����" + b;
						PCBArray[a].BlockReason = PCBArray[a].BlockReason + "�豸ʧ��";
						PCBArray[a].PC = PCBArray[a].PC - 4;
					}
					else
					{
						PCBArray[a].DN = Dev->Allocate(b);
						PCBArray[a].HaveDevice = b;
						PCBArray[a].NeedDevice = no;
						PCBArray[a].GetDeviceTime = XDTime;
						PCBArray[a].WaitTime = time;
						PCBArray[a].BlockReason = "�ȴ�IO�������";
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
					//�޸Ľ���״̬
					//
					InsertOneToBlock(a);
					Execute = 10;
					//
					//ת����̵���
					//
					int c = JudgeAttemper();
					if (c < 10)
					{
						Attemper(c);
					}

				};
				//
				//WakeUp���������ѽ���
				//
				void WakeUp(int ID, DeviceType device)
				{
					//
					//�����Լ�
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
					//����һ���ڵ�
					//
					while (Block < 10 && PCBArray[Block].NeedDevice == device)
					{
						int h = Block;
						Block = PCBArray[h].Next;
						InsertOneToReady(h);
					}
					//
					//��������ڵ�
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
				//�ж��Ƿ���Ҫ���н��̵ĵ��ȣ�����Ҫ�򷵻ؽ��̿�ţ�0-9��������Ҫ�򷵻�10
				//
				int JudgeAttemper()
				{
					//
					//ѡ���������������ȼ����
					//
					int k;
					if (Ready < 10)
					{
						int p = Ready;
						int a = PCBArray[Ready].Pri;
						k = Ready;                                      //���ȼ���ߵĿ��
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
					//��ִ�������ڵ�PCB����бȽ�
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
				//���̵��Ⱥ���
				//
				void Attemper(int a)
				{
					//
					//����CPU�ֳ�
					//
					if (Execute < 10)
					{
						PCBArray[Execute].PC = PC;
						PCBArray[Execute].IR = IR;
						PCBArray[Execute].DR = DR;
						InsertOneToReady(Execute);
					}
					//
					//ѡ��һ�����̣���ʼ��CPU�еļĴ���
					//

					GetOneFromReady(a);
					Execute = a;
					PC = PCBArray[a].PC;
					IR = PCBArray[a].IR;
					DR = PCBArray[a].DR;

				};
				//
				//������cpu
				//
				void cpu()
				{

					
				};
		private:
			void Init()
			{
				//
				//��ʼ��PCB��
				//
				White = 0;
				Ready = Block = Execute = 10;
				for (int i = 0; i < 10; i++)
				{
					PCBArray[i].ProcessID = i;
					PCBArray[i].Next = i + 1;
				}
				//
				//��ʼ���Ĵ���
				//
				PC = 0;
				PSW = No;
				PSW1 = No;
				IR = "";
				//
				//��ʼ��ʱ��
				//
				//XTTime = Convert.ToDateTime("00:00:00");
				XDTime = 0;
			}
			//
			//���캯��
			//
				//
				//�����豸���¼���ί��
				//
			
			//
			//����PCB����Ĳ���
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
