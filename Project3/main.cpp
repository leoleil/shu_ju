// Server.cpp : Defines the entry point for the console application.  
//  
#include <iostream> 
#include <thread> //thread 头文件,实现了有关线程的类
#include "winsock2.h"  
#include "MessageParser.h"
#include "Database.h"
#include "socket.h"
#include "MySQLInterface.h"
using namespace std;

vector <message_byte> MESSAGE_VECTOR;//全局报文池
CRITICAL_SECTION g_CS;//全局关键代码段对象
int STOP = 0;//是否停止socket程序运行

template <class Type> string numToString(Type& num)
{
	stringstream ss;
	ss << num;
	string s1 = ss.str();
	return s1;
}
template <class Type> Type stringToNum(const string& str)
{
	istringstream iss(str);
	Type num;
	iss >> num;
	return num;
}

int main(int argc, char* argv[])
{
	
	InitializeCriticalSection(&g_CS);//初始化关键代码段对象
    //创建线程
	HANDLE hThread1;
	HANDLE hThread2;
	hThread1 = CreateThread(NULL, 0, messagePasing, NULL, 0, NULL);
	hThread2 = CreateThread(NULL, 0, SocketServicr, NULL, 0, NULL);
	CloseHandle(hThread1);
	CloseHandle(hThread2);
	
	while (1) {

	}

	Sleep(4000);//主线程函数静默4秒
	DeleteCriticalSection(&g_CS);//删除关键代码段对象
	
	return 0;
}