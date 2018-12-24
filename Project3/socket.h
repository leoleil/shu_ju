#pragma once
#include <winsock2.h> 
#include "MessageParser.h"
#pragma comment(lib, "ws2_32.lib")   
using namespace std;

extern  vector <message_byte> MESSAGE_VECTOR;//报文池
extern CRITICAL_SECTION g_CS;//全局关键代码段对象
extern int STOP;//停止标识

//通信接口;
DWORD WINAPI SocketServicr(LPVOID lpParameter);
//报文分割，将报文分割加入报文池中
int messageCut(BYTE * message, int length);
//加入报文池
void addMessageSet(message_byte message);