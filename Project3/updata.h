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

/*�������ģ��*/
//��������߳����
DWORD WINAPI updata(LPVOID lpParameter);

