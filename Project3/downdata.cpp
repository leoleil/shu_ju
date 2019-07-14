#include "downdata.h"
vector<message_buf> DATA_MESSAGES;//全局上传数据报文池
CRITICAL_SECTION data_CS;//数据线程池关键代码段对象

DWORD download_rec(LPVOID lpParameter)
{
	InitializeCriticalSection(&data_CS);//初始化关键代码段对象
	HANDLE hThread1;//创建数据解析线程，读取数据池中数据
	hThread1 = CreateThread(NULL, 0, download, NULL, 0, NULL);
	CloseHandle(hThread1);
	while (1) {
		DowndataSocket service;//创建接收任务服务
		service.createReceiveServer(4997, DATA_MESSAGES);
	}
	DeleteCriticalSection(&data_CS);//删除关键代码段对象
	return 0;
}

DWORD download(LPVOID lpParameter)
{
	//数据库连接关键字
	const char SERVER[10] = "127.0.0.1";
	const char USERNAME[10] = "root";
	const char PASSWORD[10] = "";
	const char DATABASE[20] = "satellite_message";
	const int PORT = 3306;
	MySQLInterface mysql;//申请数据库连接对象
	//连接数据库
	if (mysql.connectMySQL(SERVER, USERNAME, PASSWORD, DATABASE, PORT)) {
		while (1) {
			Sleep(100);
			EnterCriticalSection(&data_CS);//进入关键代码段
			if (DATA_MESSAGES.empty()) {
				LeaveCriticalSection(&data_CS);
				continue;
			}
			//报文集合大小
			int setLen = DATA_MESSAGES.size();
			LeaveCriticalSection(&data_CS);//离开关键代码段

			for (int i = 0; i < setLen; i++) {
				EnterCriticalSection(&data_CS);//进入关键代码段
				char byte_data[70 * 1024];//每个报文空间最大70K
				memcpy(byte_data, DATA_MESSAGES[i].val, 70 * 1024);//将报文池中数据取出
				LeaveCriticalSection(&data_CS);//离开关键代码段
				string ackSql = "";
				DownMessage downMessage;
				downMessage.messageParse(byte_data);//解析数据
				char fileName[32];
				downMessage.getterFileName(fileName);
				char expandName[8];
				downMessage.getterExpandName(expandName);
				UINT32 taskNum = downMessage.getterTaskNum();//获取任务编号
				int size = 64 * 1024;
				char* data = new char[size];
				downMessage.getterData(data, size);//获取数据
				string taskNumFile = to_string(taskNum);
				string path = "D:\\卫星星座运管系统\\数据下行\\" + taskNumFile;
				string file_path = path + "\\" + fileName + expandName;
				//string command;
				//command = "mkdir - p " + path;
				//system(command.c_str());//创建文件夹
				if (_access(path.c_str(), 0) == -1) {	//如果文件夹不存在
					_mkdir(path.c_str());				//则创建
					cout << "| 数据下行保存路径 | " + path << endl;

				}
				//打开文件
				ofstream ofs(file_path, ios::binary | ios::out | ios::app);
				ofs.write(data, size);
				ofs.close();
				delete data;


				//判断是否是文件尾
				if (downMessage.getterEndFlag()) {
					//是文件尾要删除缓存数据
					DATA_MESSAGES.erase(DATA_MESSAGES.begin(), DATA_MESSAGES.begin() + i + 1);
					ackSql = "update 任务分配表 set 分发标志 = 1 where 任务编号 = " + taskNumFile;
					mysql.writeDataToDB(ackSql);
					cout << "| 数据下行         | 已缓存文件下载完毕" << endl;
					break;//跳出循环
				}
				else {
					//不是文件尾要查看报文池是否有更新
					//如果已经筛查到最后一个数据还没到文件尾
					if (i == setLen - 1) {
						int count = 0;
						//等待新的数据,最多一分钟
						while (count<60) {
							EnterCriticalSection(&data_CS);//进入关键代码段
							if (DATA_MESSAGES.size() > setLen) {
								int setLen = DATA_MESSAGES.size();
								break;//新数据到达跳出循环
							}
							LeaveCriticalSection(&data_CS);
							count++;
							Sleep(1000);
						}
						if (count == 60) {
							cout << "| 数据下行         | 文件上传数据等待超时" << endl;
							//删除缓存数据
							DATA_MESSAGES.erase(DATA_MESSAGES.begin(), DATA_MESSAGES.begin() + i + 1);
							//删除已经下载数据
							remove(file_path.c_str());
							cout << "| 数据下行         | 清空文件已缓存文件" << endl;
							ackSql = "update 任务分配表 set 任务状态 = 6 , ACK = 1100,任务结束时间 = now() where 任务编号 = " + taskNumFile;
							mysql.writeDataToDB(ackSql);
							break;//跳出循环
						}
					}

				}
			}

		}
	}
	else {
		cout << "| 数据下行         | 连接数据库失败" << endl;
		cout << "| 数据下行错误信息 | " << mysql.errorNum << endl;
	}
	return 0;
}
