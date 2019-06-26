#define _CRT_SECURE_NO_WARNINGS 1
#include "MessageParser.h"
#include "MySQLInterface.h"

string getTime()//获取时间字符化输出
{
	time_t timep;
	time(&timep); //获取time_t类型的当前时间
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));//对日期和时间进行格式化
	return tmp;
}
template <class Type> string numToString(Type& num)
{
	stringstream ss;
	ss << num;
	string s1 = ss.str();
	return s1;
}
//模板函数：将string类型变量转换为常用的数值类型（此方法具有普遍适用性）  
template <class Type> Type stringToNum(const string& str)
{
	istringstream iss(str);
	Type num;
	iss >> num;
	return num;
}
string getType(UINT16 num) {
	switch (num)
	{
	case 1:
		return "INT";
	case 2:
		return "INT";
	case 3:
		return "INT";
	case 4:
		return "INT";
	case 5:
		return "BIGINT";
	case 6:
		return "BIGINT";
	case 7:
		return "FLOAT";
	case 8:
		return "DOUBLE";
	case 9:
		return "VARCHAR(1)";
	case 10:
		return "VARCHAR(255)";
	case 11:
		return "VARCHAR(255)";
	case 12:
		return "BLOB)";
	case 13:
		return "BLOB";
	case 14:
		return "BLOB";
	case 15:
		return "BOOL";
	case 16:
		return "BIGINT";
	default:
		return "";
	}
}
DWORD WINAPI messagePasing(LPVOID lpParameter) {
	unordered_map<string, int> MAP;//用来标记第一个来的数据
	int db_da_flag = 1;//第一个数据报文来的标志
	//连接数据库测试

	MySQLInterface* db_test = new MySQLInterface;
	if (db_test->connectMySQL("127.0.0.1", "root", "", "teleinfodb", 3306)) {
		
		db_test->writeDataToDB("CREATE TABLE `字段定义表` (`主键` int(11) NOT NULL AUTO_INCREMENT,`字段名` varchar(20) NOT NULL,`数据类型` int(11) NOT NULL,`最小值` double DEFAULT NULL,`最大值` double DEFAULT NULL,`单位` varchar(10) NOT NULL,`显示标志` int(11) NOT NULL,`设备名` varchar(20) NOT NULL,PRIMARY KEY(`主键`))");
		db_test->writeDataToDB("CREATE TABLE `设备关系表` (`主键` int(11) NOT NULL AUTO_INCREMENT,`设备名` varchar(20) NOT NULL,`父设备名` varchar(20) DEFAULT NULL,PRIMARY KEY(`主键`))");
		db_test->closeMySQL();
	}
	else {
		//连接失败
		cout << db_test->errorNum << ":" << db_test->errorInfo << endl;
		return 0;
	}
	cout << "messagePasing starting..." << endl;
	
	while (1) {
		//Sleep(10);
		EnterCriticalSection(&g_CS);//进入关键代码段
		//监测是否有报文资源资源
		if (MESSAGE_VECTOR.empty()) {
			LeaveCriticalSection(&g_CS);//离开关键代码段
			continue;
		}
		LeaveCriticalSection(&g_CS);//离开关键代码段
		//遍历资源集合
		//遍历需要考虑到每次都能遍历整个集合
		//cout << "paring..." << endl;
		EnterCriticalSection(&g_CS);//进入关键代码段
		//报文集合大小
		int setLen = MESSAGE_VECTOR.size();
		LeaveCriticalSection(&g_CS);//离开关键代码段
		for (int i = 0; i < setLen; i++) {
			//Sleep(1);
			EnterCriticalSection(&g_CS);//进入关键代码段
			BYTE byte_data[70*K];
			memcpy(byte_data, MESSAGE_VECTOR[i].val, 70*K);//将报文池中数据取出
			//byte_data = MESSAGE_VECTOR[i].val;
			LeaveCriticalSection(&g_CS);//离开关键代码段
			//判断报文
			UINT16 type;//报文类型
			UINT16 length;//报文长度
			memcpy(&type, byte_data, 2);
			memcpy(&length, byte_data + 2, 2);
			//cout << "type is :" << type << endl;
			//#####################报文解析###############################
			
			if (type == 1000) {//定义报文
				//解析定义报文
				//解析指针
				int ptr = 4;//已经解析前两个字节
				//时间戳
				long long timestamp;
				memcpy(&timestamp, byte_data + ptr, sizeof(long long));
				ptr = ptr + sizeof(long long);
				//加密标识
				bool flag;
				memcpy(&flag, byte_data + ptr, 1);
				ptr = ptr + 1;
				//设备名
				char name[40];
				memcpy(name, byte_data + ptr, 40);
				//写日志
				MySQLInterface* db_test = new MySQLInterface;
				string sql_difinition = "insert into 系统日志表 (对象,事件类型,参数) values ('通信模块',1020,'通信中心机收到";
				sql_difinition = sql_difinition + name + "遥测报表定义报文');";
				if (db_test->connectMySQL("127.0.0.1", "root", "", "mangeinfodb", 3306)) {
					db_test->writeDataToDB(sql_difinition);
					db_test->closeMySQL();
				}
				else {
					//连接失败
					cout << db_test->errorNum << ":" << db_test->errorInfo << endl;
					return 0;
				}
				cout << "| 通信模块 | ";
				cout << getTime();
				cout << "| 接收定义报文";
				cout.setf(ios::left);
				cout.width(37);
				cout << name;
				cout << "|" << endl;
				MAP[name] = 0;//标记设备
				ptr = ptr + 40;
				//父设备名
				char parentName[40];
				memcpy(parentName, byte_data + ptr, 40);
				ptr = ptr + 40;
				//识别字段名称
				vector<field> fields;
				while (ptr < length) {
					field sub_field;
					//字段名
					memcpy(sub_field.name, byte_data + ptr, 40);
					ptr = ptr + 40;
					//字段类型编号
					memcpy(&sub_field.type, byte_data + ptr, 2);
					ptr = ptr + 2;
					//最小值
					memcpy(&sub_field.min, byte_data + ptr, 8);
					ptr = ptr + 8;
					//最大值
					memcpy(&sub_field.max, byte_data + ptr, 8);
					ptr = ptr + 8;
					//单位
					memcpy(sub_field.unit, byte_data + ptr, 8);
					ptr = ptr + 8;
					//显示标识
					memcpy(&sub_field.display, byte_data + ptr, 1);
					ptr = ptr + 1;
					fields.push_back(sub_field);
				}
				//入库操作
				MySQLInterface* db = new MySQLInterface;
				//连接数据库
				if (db->connectMySQL("127.0.0.1", "root", "", "teleinfodb", 3306)) {
					//查询是否有该定义字段
					string f_sql = "select * from 字段定义表 where 设备名 =";
					f_sql = f_sql + "'" + name + "';";
					//cout << f_sql << endl;
					vector<vector <string>>res;
					if (!db->getDatafromDB(f_sql, res)) {
						cout << db->errorNum << endl;
						cout << db->errorInfo << endl;
					}
					if (res.size()!=0) {
						cout << name << "设备在定义表中已经存在" << endl;
					}
					else {
						//不存在则创建
						
						//创建字段定义
						for (int i = 0; i < fields.size(); i++) {
							//创建定义的sql语句
							string sql = "insert into 字段定义表 (字段名,数据类型,最大值,最小值,单位,显示标志,设备名) values(";
							sql = sql + "'" + fields[i].name + "',";
							sql = sql + numToString<UINT16>(fields[i].type) + ",";
							//如果没有最大最小值
							if (fields[i].max == NEW_NULL) {
								sql = sql + "NULL,";
							}
							else {
								sql=sql+ numToString<double>(fields[i].max) + ",";
							}
							if (fields[i].min == NEW_NULL) {
								sql = sql + "NULL,";
							}
							else {
								sql = sql + numToString<double>(fields[i].min) + ",";
							}
							
							sql = sql + "'" + fields[i].unit + "',";
							sql = sql + numToString<bool>(fields[i].display) + ",";
							sql = sql + "'" + name + "');";
							//cout << sql << endl;
							if (!db->writeDataToDB(sql)) {
								cout << db->errorNum << endl;
								cout << db->errorInfo << endl;
							}
						}
						//创建该数据的表
						string d_sql = "create table ";
						d_sql = d_sql + name + " (主键 INT AUTO_INCREMENT,时间 BIGINT,";
						for (int i = 0; i < fields.size(); i++) {
							d_sql = d_sql + fields[i].name + " " + getType(fields[i].type);
							if (i != fields.size() - 1)d_sql = d_sql + ",";
						}
						d_sql = d_sql + ",primary key(主键));";
						//cout << d_sql << endl;
						if (!db->writeDataToDB(d_sql)) {
							//cout << db->errorNum << endl;
							//cout << db->errorInfo << endl;
						}
						//创建关系表
						string r_sql = "insert into 设备关系表(设备名,父设备名) values('";
						r_sql = r_sql + name + "','" + parentName + "');";
                        if (!db->writeDataToDB(r_sql)) {
							//cout << db->errorNum << endl;
							//cout << db->errorInfo << endl;
						}
						//Sleep(10);
						//将该报文删除
						EnterCriticalSection(&g_CS);//进入关键代码段
													//报文池-1
						MESSAGE_VECTOR.erase(MESSAGE_VECTOR.begin() + i);
						i--;
						LeaveCriticalSection(&g_CS);//离开关键代码段

					}
					//关闭连接
					db->closeMySQL();
				}
				else {
					//cout << db->errorNum << endl;
					//cout << db->errorInfo << endl;
				}
				
			}
			else if (type == 2000) {
				//解析数据报文
				//解析指针
				int ptr = 4;//已经解析两个字节
				//时间戳
				long long timestamp;
				memcpy(&timestamp, byte_data + ptr, sizeof(long long));
				ptr = ptr + sizeof(long long);
				//加密标识
				bool flag;
				memcpy(&flag, byte_data + ptr, sizeof(bool));
				ptr = ptr + sizeof(bool);
				//设备名
				char name[40];
				memcpy(name, byte_data + ptr, 40);
				//如果是第一个数据要入库
				if (MAP[name] == 0) {
					MAP[name] = 1;
					MySQLInterface* db_test = new MySQLInterface;
					string sql = "insert into 系统日志表 (对象,事件类型,参数) values ('通信模块',1021,'通信中心机收到第一份";
					sql = sql + name + "遥测报表数据报文');";
					cout << "| 通信模块 | ";
					cout << getTime();
					cout << "| 接收数据报文";
					cout.setf(ios::left);
					cout.width(37);
					cout << name;
					cout << "|" << endl;
					if (db_test->connectMySQL("127.0.0.1", "root", "", "mangeinfodb", 3306)) {
						db_test->writeDataToDB(sql);
						db_test->closeMySQL();
					}
					else {
						//连接失败
						//cout << db_test->errorNum << ":" << db_test->errorInfo << endl;
						return 0;
					}
				}
				ptr = ptr + 40;

				//如果数据库有此字段
				//识别数据入库
				MySQLInterface* db = new MySQLInterface;
				//连接数据库
				if (db->connectMySQL("127.0.0.1", "root", "", "teleinfodb", 3306)) {
					
					//入库操作
					//查询字段数据类型
					vector<vector <string>>res;
					string ff_sql = "select 数据类型,字段名 from 字段定义表 where 设备名=";
					ff_sql = ff_sql + "'" + name + "' order by 主键;";
					if (db->getDatafromDB(ff_sql, res)) {
						if (res.size() == 0) {
							cout << name << "设备不存在" << endl;
						}
						//数据库中数据
						else {
							string sql = "insert into ";
							sql = sql + name + "(时间,";
							string d_sql = " values(";
							d_sql = d_sql + numToString<long long>(timestamp) + ",";
							for (int i = 0; i < res.size(); i++) {
								//类型号码
								UINT16 code = 0;
								code = stringToNum<UINT16>(res[i][0]);
								sql = sql + res[i][1];
								if (i < res.size() - 1)sql = sql + ",";
								if (i == res.size() - 1)sql = sql + ")";
								if (code == 1) {//16位整数
									INT16 data = 0;
									memcpy(&data, byte_data + ptr, sizeof(INT16));
									ptr = ptr + sizeof(INT16);
									d_sql = d_sql + numToString<INT16>(data);
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 2) {//16位无符号整数
									UINT16 data = 0;
									memcpy(&data, byte_data + ptr, sizeof(UINT16));
									ptr = ptr + sizeof(UINT16);
									d_sql = d_sql + numToString<UINT16>(data);
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 3) {//32位整数
									INT32 data = 0;
									memcpy(&data, byte_data + ptr, sizeof(INT32));
									ptr = ptr + sizeof(INT32);
									d_sql = d_sql + numToString<INT32>(data);
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 4) {//32位无符号整数
									UINT32 data = 0;
									memcpy(&data, byte_data + ptr, sizeof(UINT32));
									ptr = ptr + sizeof(UINT32);
									d_sql = d_sql + numToString<UINT32>(data);
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 5) {//64位整数
									INT64 data = 0;
									memcpy(&data, byte_data + ptr, sizeof(INT64));
									ptr = ptr + sizeof(INT64);
									d_sql = d_sql + numToString<INT64>(data);
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 6) {//64位无符号整数
									UINT64 data = 0;
									memcpy(&data, byte_data + ptr, sizeof(UINT64));
									ptr = ptr + sizeof(UINT64);
									d_sql = d_sql + numToString<UINT64>(data);
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 7) {//单精度浮点数值
									float data = 0;
									memcpy(&data, byte_data + ptr, sizeof(float));
									ptr = ptr + sizeof(float);
									d_sql = d_sql + numToString<float>(data);
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 8) {//双精度浮点数值
									double data = 0;
									memcpy(&data, byte_data + ptr, sizeof(double));
									ptr = ptr + sizeof(double);
									d_sql = d_sql + numToString<double>(data);
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 9) {//字符类型
									char data;
									memcpy(&data, byte_data + ptr, sizeof(char));
									ptr = ptr + sizeof(char);
									d_sql = d_sql + "'" +data + "'";
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 10) {//短字符串类型
									char data[15];
									memcpy(&data, byte_data + ptr, sizeof(char) * 15);
									ptr = ptr + sizeof(char) * 15;
									d_sql = d_sql + "'" + data + "'";
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 11) {//长字符串类型
									char data[255];
									memcpy(&data, byte_data + ptr, sizeof(char) * 255);
									ptr = ptr + sizeof(char) * 255;
									d_sql = d_sql + "'" + data + "'";
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								//有bug
								else if (code == 12) {//短字节数组
									char data[255];
									memcpy(&data, byte_data + ptr, sizeof(char) * 255);
									ptr = ptr + sizeof(char) * 255;
									d_sql = d_sql + "'" + data + "'";
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 13) {//中字节数组
									char data[32 * K];
									memcpy(&data, byte_data + ptr, sizeof(char) * 32 * K);
									ptr = ptr + sizeof(char) * 32 * K;
									d_sql = d_sql + "'" + data + "'";
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 14) {//长字节数组
									char data[60 * K];
									memcpy(&data, byte_data + ptr, sizeof(char) * 60 * K);
									ptr = ptr + sizeof(char) * 60 * K;
									d_sql = d_sql + "'" + data + "'";
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 15) {//布尔型
									bool data;
									memcpy(&data, byte_data + ptr, sizeof(bool));
									ptr = ptr + sizeof(bool);
									d_sql = d_sql + numToString<bool>(data);
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 16) {//时间戳
									long long data;
									memcpy(&data, byte_data + ptr, sizeof(long long));
									ptr = ptr + sizeof(long long);
									d_sql = d_sql + numToString<long long>(data);
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else {
									cout << "未定义数据类型错误" << endl;
								}
							}
							d_sql = d_sql + ");";
							sql = sql + d_sql;
							//cout << sql << endl;
							if (!db->writeDataToDB(sql)) {
								//cout << db->errorNum << endl;
								//cout << db->errorInfo << endl;
							}
							//Sleep(10);
							EnterCriticalSection(&g_CS);//进入关键代码段
														//报文池-1
							MESSAGE_VECTOR.erase(MESSAGE_VECTOR.begin() + i);
							i--;
							LeaveCriticalSection(&g_CS);//离开关键代码段
						}
						
					}
					else {
						//cout << db->errorNum << endl;
						//cout << db->errorInfo << endl;
					}
					//关闭连接
					db->closeMySQL();
				}
				else {
					//cout << db->errorNum << endl;
					//cout << db->errorInfo << endl;
				}
			}
			else {
				EnterCriticalSection(&g_CS);//进入关键代码段
				STOP = 1;
				LeaveCriticalSection(&g_CS);//离开关键代码段
				//错误处理
				//cout << "未知报文格式" << endl;
			}
			
			//更新报文池的大小
			//Sleep(1);
			EnterCriticalSection(&g_CS);//进入关键代码段
			setLen =(int) MESSAGE_VECTOR.size();
			//system("cls");
			//cout << "目前报文池大小：" << MESSAGE_VECTOR.size() << endl;
			LeaveCriticalSection(&g_CS);//离开关键代码段
		}
		
	}
	return 0;
}


