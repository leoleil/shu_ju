#pragma once
#include <iostream>  
#include <sstream>
#include <string>
#include <fstream>  
#include <vector>
#include <unordered_map>
#include "winsock2.h" 
#include <time.h>
#include <Windows.h>
#include "MySQLInterface.h"
#include "StringNumUtils.h"
#include "DowndataSocket.h"
#include <thread> //thread 头文件,实现了有关线程的类
#include "DownMessage.h"
#include <io.h>
#include <direct.h>

using namespace std;

extern string MYSQL_SERVER;
extern string MYSQL_USERNAME;
extern string MYSQL_PASSWORD;

//数据上传线程
DWORD WINAPI download_rec(LPVOID lpParameter);
DWORD WINAPI download(LPVOID lpParameter);