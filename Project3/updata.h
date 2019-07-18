#pragma once
#include <iostream>  
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include "winsock2.h" 
#include <time.h>
#include <Windows.h>
#include "MySQLInterface.h"
#include "Message.h"
#include "UpMessage.h"
#include "StringNumUtils.h"
#include "Socket.h"

using namespace std;
extern string MYSQL_SERVER;
extern string MYSQL_USERNAME;
extern string MYSQL_PASSWORD;

/*任务分配模块*/
//任务分配线程入口
DWORD WINAPI updata(LPVOID lpParameter);


