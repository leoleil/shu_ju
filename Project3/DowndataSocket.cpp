#include "DowndataSocket.h"
#include <iostream> 
#include <sstream>
#include <string>
#include "MySQLInterface.h"
using namespace std;
extern string MYSQL_SERVER;
extern string MYSQL_USERNAME;
extern string MYSQL_PASSWORD;
extern CRITICAL_SECTION data_CS;//ȫ�ֹؼ�����ζ���
extern vector<message_buf> DATA_MESSAGES;//ȫ���ϴ����ݱ��ĳ�


DowndataSocket::DowndataSocket()
{
}


DowndataSocket::~DowndataSocket()
{
}

int DowndataSocket::createReceiveServer(const int port, std::vector<message_buf>& message)
{
	const char* SERVER = MYSQL_SERVER.data();//���ӵ����ݿ�ip
	const char* USERNAME = MYSQL_USERNAME.data();
	const char* PASSWORD = MYSQL_PASSWORD.data();
	const char DATABASE[20] = "uav_message";
	const int PORT = 3306;
	
	cout << "| ��������         | ��������" << endl;
	//��ʼ���׽��ֶ�̬��  
	if (WSAStartup(MAKEWORD(2, 2), &S_wsd) != 0)
	{
		cout << "WSAStartup failed!" << endl;
		return 1;
	}

	//�����׽���  
	sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sServer)
	{
		cout << "socket failed!" << endl;
		WSACleanup();//�ͷ��׽�����Դ;  
		return  -1;
	}

	//�������׽��ֵ�ַ   
	addrServ.sin_family = AF_INET;
	addrServ.sin_port = htons(port);
	addrServ.sin_addr.s_addr = INADDR_ANY;
	//���׽���  
	retVal = bind(sServer, (LPSOCKADDR)&addrServ, sizeof(SOCKADDR_IN));
	if (SOCKET_ERROR == retVal)
	{
		cout << "bind failed!" << endl;
		closesocket(sServer);   //�ر��׽���  
		WSACleanup();           //�ͷ��׽�����Դ;  
		return -1;
	}

	//��ʼ����   
	cout << "| ��������         | listening" << endl;
	retVal = listen(sServer, 1);

	if (SOCKET_ERROR == retVal)
	{
		cout << "listen failed!" << endl;
		closesocket(sServer);   //�ر��׽���  
		WSACleanup();           //�ͷ��׽�����Դ;  
		return -1;
	}

	//���ܿͻ�������  
	sockaddr_in addrClient;
	int addrClientlen = sizeof(addrClient);
	sClient = accept(sServer, (sockaddr FAR*)&addrClient, &addrClientlen);
	if (INVALID_SOCKET == sClient)
	{
		cout << "accept failed!" << endl;
		closesocket(sServer);   //�ر��׽���  
		WSACleanup();           //�ͷ��׽�����Դ;  
		return -1;
	}

	cout << "| ��������         | TCP���Ӵ���" << endl;
	//д��־
	MySQLInterface mysql;
	if (mysql.connectMySQL(SERVER, USERNAME, PASSWORD, DATABASE, PORT)) {
		string logSql = "insert into ϵͳ��־�� (ʱ��,����,�¼�����,�¼�˵��) values (now(),'��������ģ��',14000,'����TCP����');";
		mysql.writeDataToDB(logSql);
		//mysql.closeMySQL();
	}
	else {
		cout << "| ��������        | ���ݿ�����ʧ��" << endl;
		return 0;
	}
	
	while (true) {
		//���ݴ���
		const int data_len = 66560;//ÿ�ν���65K���ݰ�
		char data[66560]; //���ݰ�
		ZeroMemory(data, data_len);//�����ݰ��ռ���0
		char* data_ptr = data;//����ָ��
		int r_len = 0;
		while (1) {
			//���տͻ�������
			//���buffer
			ZeroMemory(buf, BUF_SIZE);

			//��ȡ����
			retVal = recv(sClient, buf, BUF_SIZE, 0);

			if (SOCKET_ERROR == retVal)
			{
				cout << "| ��������         | ���ճ������" << endl;
				closesocket(sServer);   //�ر��׽���    
				closesocket(sClient);   //�ر��׽���
				return -1;
			}
			if (retVal == 0) {
				cout << "| ��������         | ������϶Ͽ���������" << endl;
				//д��־
				string logSql = "insert into ϵͳ��־�� (ʱ��,����,�¼�����,�¼�˵��) values (now(),'��������ģ��',14001,'�Ͽ�TCP����');";
				mysql.writeDataToDB(logSql);
				mysql.closeMySQL();
				
				closesocket(sServer);   //�ر��׽���    
				closesocket(sClient);   //�ر��׽���
				return -1;
			}
			Sleep(10);
			memcpy(data_ptr, buf, retVal);
			r_len = r_len + retVal;

			data_ptr = data_ptr + retVal;
			if ((data_ptr - data) >= data_len) {
				break;//������յ������ݴ�����󴰿�����ѭ�������ݰ����64K��
			}

		}


		//����ȡ�������ݷ������ݳ���
		char* ptr = data;
		UINT32 length = 0;
		for (int i = 0; i < data_len; i = i + length) {

			if (data[i] == NULL && data[i + 1] == NULL)break;
			//��ȡ���ĳ���
			memcpy(&length, ptr + i + 2, 4);

			//��ȡһ��buffer
			message_buf messageBuf;
			//�ڴ渴��
			memcpy(&messageBuf.val, ptr + i, length);
			//���뱨�ĳ�
			EnterCriticalSection(&data_CS);//����ؼ������
			message.push_back(messageBuf);
			LeaveCriticalSection(&data_CS);//�뿪�ؼ������

		}

	}
	//�˳�  
	closesocket(sServer);   //�ر��׽���  
	closesocket(sClient);   //�ر��׽���  
	return 0;
}
