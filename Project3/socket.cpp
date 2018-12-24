#include"socket.h"
#include "MySQLInterface.h"
const int BUF_SIZE = 10*M;			//接收buffer大小
WSADATA         wsd;            //WSADATA变量  
SOCKET          sServer;        //服务器套接字  
SOCKET          sClient;        //客户端套接字  
SOCKADDR_IN     addrServ;;      //服务器地址  
char            buf[BUF_SIZE];  //接收数据缓冲区  
char            sendBuf[BUF_SIZE];//返回给客户端得数据  
int             retVal;         //返回值  

DWORD WINAPI SocketServicr(LPVOID lpParameter)
{

	//初始化套结字动态库  
	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		cout << "WSAStartup failed!" << endl;
		return 1;
	}

	//创建套接字  
	sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sServer)
	{
		cout << "socket failed!" << endl;
		WSACleanup();//释放套接字资源;  
		return  -1;
	}

	//服务器套接字地址   
	addrServ.sin_family = AF_INET;
	addrServ.sin_port = htons(4999);
	addrServ.sin_addr.s_addr = INADDR_ANY;
	//绑定套接字  
	retVal = bind(sServer, (LPSOCKADDR)&addrServ, sizeof(SOCKADDR_IN));
	if (SOCKET_ERROR == retVal)
	{
		cout << "bind failed!" << endl;
		closesocket(sServer);   //关闭套接字  
		WSACleanup();           //释放套接字资源;  
		return -1;
	}

	//开始监听   
	cout << "listening ..." << endl;
	retVal = listen(sServer, 1);
	
	if (SOCKET_ERROR == retVal)
	{
		cout << "listen failed!" << endl;
		closesocket(sServer);   //关闭套接字  
		WSACleanup();           //释放套接字资源;  
		return -1;
	}

	//接受客户端请求  
	sockaddr_in addrClient;
	int addrClientlen = sizeof(addrClient);
	sClient = accept(sServer, (sockaddr FAR*)&addrClient, &addrClientlen);
	if (INVALID_SOCKET == sClient)
	{
		cout << "accept failed!" << endl;
		closesocket(sServer);   //关闭套接字  
		WSACleanup();           //释放套接字资源;  
		return -1;
	}
	cout << "accept success!" << endl;
	cout << "data from cline" << endl;
	MySQLInterface* db_test = new MySQLInterface;
	if (db_test->connectMySQL("127.0.0.1", "root", "", "mangeinfodb", 3306)) {
		db_test->writeDataToDB("insert into 系统日志表 (对象,事件类型,参数) values ('通信模块',1010,'本地与发送端建立TCP连接');");
		db_test->closeMySQL();
	}
	else {
		//连接失败
		cout << db_test->errorNum << ":" << db_test->errorInfo << endl;
		return 0;
	}
	while (true)
	{
		EnterCriticalSection(&g_CS);//进入关键代码段
		if (STOP == 1)break;
		LeaveCriticalSection(&g_CS);//离开关键代码段
		//接收客户端数据
		//清空buffer
		ZeroMemory(buf, BUF_SIZE);
		//获取数据
		retVal = recv(sClient, buf, BUF_SIZE, 0);
		
		if (SOCKET_ERROR == retVal)
		{
			cout << "recv failed!" << endl;
			closesocket(sServer);   //关闭套接字
			closesocket(sClient);   //关闭套接字       
			WSACleanup();           //释放套接字资源;  
			return -1;
		}
		cout << "获取" << retVal << "字节" << endl;
		
		Sleep(10);
		
		//将buf里的字节拷贝
		BYTE byte_buf[BUF_SIZE];
		memcpy(byte_buf, buf, BUF_SIZE);
		
		//调用报文解析接口
		messageCut(byte_buf, BUF_SIZE);
		
	}
	if (db_test->connectMySQL("127.0.0.1", "root", "", "mangeinfodb", 3306)) {
		db_test->writeDataToDB("insert into 系统日志表 (对象,事件类型,参数) values ('本地与发送端断开TCP连接');");
		db_test->closeMySQL();
	}
	else {
		//连接失败
		cout << db_test->errorNum << ":" << db_test->errorInfo << endl;
		return 0;
	}
	system("cls");
	cout << "服务关闭,数据处理结束" << endl;
	//退出  
	closesocket(sServer);   //关闭套接字  
	closesocket(sClient);   //关闭套接字  
	WSACleanup();           //释放套接字资源;  
	return 0;
}
//获取报文子串并加入报文池中
int messageCut(BYTE * message, int size) {
	BYTE * ptr = message;
	UINT16 length=0;
	for (int i = 0; i < size - 1; i = i + length) {

		if (ptr[i] == 0X00 && ptr[i + 1] == 0X00)return 0;
		//获取报文长度
		memcpy(&length, ptr + i + 2, 2);
		//cout << "len:" << length << endl;
		//获取一个buffer
		message_byte buffer;
		//内存复制
		memcpy(buffer.val, ptr + i, length);
		//加入报文池
		//Sleep(5);
		EnterCriticalSection(&g_CS);//进入关键代码段
		MESSAGE_VECTOR.push_back(buffer);
		//cout << "目前MESSAGE_VECTOR：" << MESSAGE_VECTOR.size() << endl;
		LeaveCriticalSection(&g_CS);//离开关键代码段
	}
	return 0;
}
