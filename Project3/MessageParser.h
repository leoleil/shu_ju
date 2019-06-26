#pragma once
#include <iostream>  
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include "winsock2.h" 
#include <time.h>
#include <Windows.h>

using namespace std;



const int M = 1048576;//1M
const int K = 1024;   //1K
const double NEW_NULL = 1000000000;//这个数代表NULL

struct message_set;
struct message_byte;
struct field;
struct data;

typedef struct message_set message_set;
typedef struct message_byte message_byte;
typedef struct field field;
typedef struct data data;



struct message_byte
{
	//70K
	BYTE val[70*K];
};

//定义报文字段
struct field
{
	char name[20];
	UINT16 type;
	double min;
	double max;
	char unit[8];
	bool display;
};
struct data
{
	BYTE byte_data[64 * K];
};

////定义一个报文池保存接收到的全部报文
//vector <message_byte> MESSAGE_VECTOR;//报文池
//CRITICAL_SECTION g_CS;//全局关键代码段对象
extern  vector <message_byte> MESSAGE_VECTOR;//报文池
extern CRITICAL_SECTION g_CS;//全局关键代码段对象
extern int STOP;//停止标识
				
string getTime();//获取时间字符化输出
//程序API

//报文解析线程入口
DWORD WINAPI messagePasing(LPVOID lpParameter);
string getType(UINT16 num);
