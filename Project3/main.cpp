// Server.cpp : Defines the entry point for the console application.  
//  
#include <iostream> 
#include <thread> //thread 头文件,实现了有关线程的类
#include "winsock2.h"  
#include "MessageParser.h"
#include "MySQLInterface.h"
#include "updata.h"
#include "downdata.h"
using namespace std;

vector <message_byte> MESSAGE_VECTOR;//全局报文池
CRITICAL_SECTION g_CS;//全局关键代码段对象
int STOP = 0;//是否停止socket程序运行
string MYSQL_SERVER = "";//连接的数据库ip
string MYSQL_USERNAME = "";
string MYSQL_PASSWORD = "";
int main(int argc, char* argv[])
{
	ifstream is("config.txt", ios::in);
	if (!is.is_open()) {
		cout << "| 数据库连接       | ";
		cout << " 无法打开配置文件，请检查config.txt是否配置，并重启系统" << endl;
		//创建不成功释放资源
		system("pause");
		return 0;
	}
	getline(is, MYSQL_SERVER);
	getline(is, MYSQL_USERNAME);
	getline(is, MYSQL_PASSWORD);
	is.close();

	InitializeCriticalSection(&g_CS);//初始化关键代码段对象
    //创建线程
	HANDLE hThread1;//数据下行线程
	hThread1 = CreateThread(NULL, 0, download_rec, NULL, 0, NULL);
	CloseHandle(hThread1);

	HANDLE hThread2;//数据上行
	hThread2 = CreateThread(NULL, 0, updata, NULL, 0, NULL);
	CloseHandle(hThread2);

	while (1) {
		if (STOP == 1)break;
	}

	Sleep(4000);//主线程函数静默4秒
	DeleteCriticalSection(&g_CS);//删除关键代码段对象
	system("pause");
	return 0;
}