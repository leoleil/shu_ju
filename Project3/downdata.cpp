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
	const char * SERVER = MYSQL_SERVER.c_str() ;
	const char * USERNAME = MYSQL_USERNAME.data();
	const char * PASSWORD = MYSQL_PASSWORD.data();
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

			while (1) {
				EnterCriticalSection(&data_CS);//进入关键代码段
				char byte_data[70 * 1024];//每个报文空间最大70K
				memcpy(byte_data, DATA_MESSAGES[0].val, 70 * 1024);//将报文池中数据取出
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
				//检查数据库中任务编号的卫星编号
				string sql = "select 卫星编号 from 任务分配表 where 任务编号 = " + taskNumFile + ";";
				vector<vector<string>> s;
				mysql.getDatafromDB(sql, s);
				string satelliteId = s[0][0];

				string path = "D:\\卫星星座运管系统\\数据下行\\" + satelliteId;
				if (_access(path.c_str(), 0) == -1) {//如果文件夹不存在
					_mkdir(path.c_str());//则创建
				}
				path = path + "\\" + taskNumFile;
				string file_path = path + "\\" + fileName + expandName;
				if (_access(path.c_str(), 0) == -1) {	
					_mkdir(path.c_str());				
					cout << "| 数据下行保存路径 | " + path << endl;

				}
				//打开文件
				ofstream ofs(file_path, ios::binary | ios::out | ios::app);
				ofs.write(data, size);
				ofs.close();
				delete data;
				EnterCriticalSection(&data_CS);//进入关键代码段
				DATA_MESSAGES.erase(DATA_MESSAGES.begin(), DATA_MESSAGES.begin() + 1);//删除元素
				LeaveCriticalSection(&data_CS);

				//判断是否是文件尾
				if (downMessage.getterEndFlag()) {
					//是文件尾要删除缓存数据
					//DATA_MESSAGES.erase(DATA_MESSAGES.begin(), DATA_MESSAGES.begin() + i + 1);
					ackSql = "update 任务分配表 set 任务状态 = 6 where 任务编号 = " + taskNumFile;
					mysql.writeDataToDB(ackSql);
					cout << "| 数据下行         | 已缓存文件下载完毕" << endl;
					string logSql = "insert into 系统日志表 (时间,对象,事件类型,参数) values (now(),'数据下行模块',14001,'数据中心机:" + taskNumFile + " 收到最后一份数据报文');";
					mysql.writeDataToDB(logSql);
					break;//跳出循环
				}
				else {
					int count = 0;
					//等待新的数据,最多一分钟
					while (count<60) {
						EnterCriticalSection(&data_CS);//进入关键代码段
						if (DATA_MESSAGES.size() > 0) {
							LeaveCriticalSection(&data_CS);
							break;//新数据到达跳出循环
						}
						LeaveCriticalSection(&data_CS);
						count++;
						Sleep(1000);
					}
					if (count == 60) {
						cout << "| 数据下行         | 文件上传数据等待超时" << endl;
						//删除缓存数据
						DATA_MESSAGES.erase(DATA_MESSAGES.begin(), DATA_MESSAGES.begin() + 1);
						//删除已经下载数据
						remove(file_path.c_str());
						cout << "| 数据下行         | 清空文件已缓存文件" << endl;
						ackSql = "update 任务分配表 set 任务状态 = 6 ,任务结束时间 = now() where 任务编号 = " + taskNumFile;
						mysql.writeDataToDB(ackSql);
						break;//跳出循环
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
