#include "updata.h"

DWORD updata(LPVOID lpParameter)
{
	//数据库连接关键字
	const char SERVER[10] = "127.0.0.1";
	const char USERNAME[10] = "root";
	const char PASSWORD[10] = "123456";
	const char DATABASE[20] = "satellite_message";
	const int PORT = 3306;
	while (1) {
		//5秒监测数据库的任务分配表
		Sleep(5000);
		cout << "| 数据上行         | 监测数据库分配表..." << endl;
		MySQLInterface mysql;//申请数据库连接对象

		//连接数据库
		if (mysql.connectMySQL(SERVER, USERNAME, PASSWORD, DATABASE, PORT)) {
			//从数据中获取分配任务
			//寻找分发标志为2，数据分发标志为0的任务
			string selectSql = "select 任务编号,任务类型,unix_timestamp(计划开始时间),unix_timestamp(计划截止时间),卫星编号,地面站编号 from 任务分配表 where 分发标志 = 2 and 任务标志 = 0 and 任务类型 = 110";
			vector<vector<string>> dataSet;
			mysql.getDatafromDB(selectSql, dataSet);
			if (dataSet.size() == 0) {
				continue;//无任务静默5秒后继续查询
			}
			//查询到任务
			for (int i = 0, len = dataSet.size(); i < len; i++) {
				char* messageDate;
				int messageDataSize = 0;
				if (!dataSet[i][1]._Equal("110")) {
					
					continue;//继续等待
				}
				//数据上行任务
				StringNumUtils util;//字符转数字工具

				long long dateTime = Message::getSystemTime();//获取当前时间戳
				bool encrypt = false;//是否加密
				UINT32 taskNum = util.stringToNum<UINT32>(dataSet[i][0]);//任务编号
				UINT16 taskType = util.stringToNum<UINT16>(dataSet[i][1]);//任务类型
				long long taskStartTime = util.stringToNum<long long>(dataSet[i][2]);//计划开始时间
				if (taskStartTime > dateTime) {
				 //如果还没到计划开始时间就跳过
					continue;
				}

				long long taskEndTime = util.stringToNum<long long>(dataSet[i][3]);//计划截止时间
				char* satelliteId = new char[20];//卫星编号
				strcpy_s(satelliteId, dataSet[i][4].size() + 1, dataSet[i][4].c_str());
				char* groundStationId = new char[20];//地面站编号
				strcpy_s(groundStationId, dataSet[i][5].size() + 1, dataSet[i][5].c_str());
				
				//查找地面站ip地址发送报文
				string groundStationSql = "select IP地址 from 地面站信息表 where 地面站编号 =" + dataSet[i][5];
				vector<vector<string>> ipSet;
				mysql.getDatafromDB(groundStationSql, ipSet);
				if (ipSet.size() == 0){
					delete groundStationId;
					delete satelliteId;
					continue;//没有找到ip地址
				}

				

				//读取数据上行配置文件
				//读取配置文件创建接收服务
				string config = "D:\\卫星星座运管系统\\数据上行\\" + dataSet[i][4] + "\\"  + dataSet[i][0] + "\\config.txt";
				ifstream is(config, ios::in);
				if (!is.is_open()) {
					cout << "| 数据上行         | " ;
					cout << config << " 无法打开" << endl;
					//创建不成功释放资源
					delete groundStationId;
					delete satelliteId;
					continue;
				}
				string fileName = "";//文件名
				string expandName = "";//扩展名
				
				getline(is, fileName);
				getline(is, expandName);
				is.close();
				string file = "D:\\卫星星座运管系统\\数据上行\\"+ dataSet[i][4] + "\\" + dataSet[i][0]+"\\" + fileName + expandName;
				ifstream fileIs(file, ios::binary | ios::in);
				if (!fileIs.is_open()) {
					cout << "| 数据上行         | ";
					cout << file << " 无法打开" << endl;
					//创建不成功释放资源
					delete groundStationId;
					delete satelliteId;
					continue;
				}
				//写日志
				string sql_date = "insert into 系统日志表 (对象,事件类型,参数) values ('数据上行模块',13000,";
				sql_date = sql_date + ipSet[0][0] + ":" + dataSet[i][0] + "与数据通信中心机创建TCP连');";
				mysql.writeDataToDB(sql_date);

				//创建发送者
				Socket socketer;
				const char* ip = ipSet[0][0].c_str();//获取到地址
													 //建立TCP连接
				if (!socketer.createSendServer(ip, 4999, 0)) {
					//创建不成功释放资源
					delete groundStationId;
					delete satelliteId;
					continue;
				}

				//读取文件
				while (!fileIs.eof()) {
					int bufLen = 1024 * 64;//数据最大64K
					char* fileDataBuf = new char[bufLen];//64K
					fileIs.read(fileDataBuf, bufLen);
					bufLen = fileIs.gcount();//获取实际读取数据大小

					char* up_file_name = new char[32];//文件名
					strcpy_s(up_file_name, fileName.size() + 1, fileName.c_str());
					char* up_expand_name = new char[8];//拓展名
					strcpy_s(up_expand_name, expandName.size() + 1, expandName.c_str());
					bool endFlag = fileIs.eof();//文件尾判断
					//创建数据上行包
					UpMessage upMessage(3020, dateTime, encrypt, taskNum, up_file_name, up_expand_name, endFlag);
					upMessage.setterData(fileDataBuf, bufLen);//传入数据
					
					const int bufSize = 66560;//发送包固定65k
					int returnSize = 0;
					char* sendBuf = new char[bufSize];//申请发送buf
					ZeroMemory(sendBuf, bufSize);//清空发送空间
					upMessage.createMessage(sendBuf, returnSize, bufSize);//创建传输字节包
					if (socketer.sendMessage(sendBuf, bufSize) == -1) {//发送包固定65k
						
						//发送失败释放资源跳出文件读写
						cout << "| 数据上行         | 发送失败，断开连接" << endl;
						sql_date = "insert into 系统日志表 (对象,事件类型,参数) values ('数据上行模块',13000,'";
						sql_date = sql_date + ipSet[0][0] + ":" + dataSet[i][0] + "与数据通信中心机断开TCP连');";
						mysql.writeDataToDB(sql_date);
						delete sendBuf;
						delete up_expand_name;
						delete up_file_name;
						delete fileDataBuf;
						break; 
					}
					//flieOs.write(fileDataBuf, bufLen);
					if (fileIs.eof()== true) {
						
						cout << "| 数据上行         | " << dataSet[i][0] << "号任务上传成功" << endl;
						sql_date = "insert into 系统日志表 (对象,事件类型,参数) values ('数据上行模块',13000,'";
						sql_date = sql_date + ipSet[0][0] + ":" + dataSet[i][0] + "与数据通信中心机断开TCP连');";
						mysql.writeDataToDB(sql_date);
						//修改数据库分发标志
						string ackSql = "update 任务分配表 set 任务标志 = 1 where 任务编号 = " + dataSet[i][0];
						mysql.writeDataToDB(ackSql);
						
					}
					
					delete sendBuf;
					delete up_expand_name;
					delete up_file_name;
					delete fileDataBuf;
					
				}
				//断开TCP
				socketer.offSendServer();
				fileIs.close();
				delete groundStationId;
				delete satelliteId;
			}
			

		}
		else {
			cout << "| 数据上行         | 连接数据库失败" << endl;
			cout << "| 数据上行错误信息 | " << mysql.errorNum << endl;
		}
		cout << "| 数据上行         | 任务结束" << endl;

	}
	return 0;
}
