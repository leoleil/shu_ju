#include "downdata.h"
vector<message_buf> DATA_MESSAGES;//ȫ���ϴ����ݱ��ĳ�
CRITICAL_SECTION data_CS;//�����̳߳عؼ�����ζ���

DWORD download_rec(LPVOID lpParameter)
{
	InitializeCriticalSection(&data_CS);//��ʼ���ؼ�����ζ���
	HANDLE hThread1;//�������ݽ����̣߳���ȡ���ݳ�������
	hThread1 = CreateThread(NULL, 0, download, NULL, 0, NULL);
	CloseHandle(hThread1);
	
	while (1) {
		DowndataSocket service;//���������������
		service.createReceiveServer(4997, DATA_MESSAGES);
	}
	
	DeleteCriticalSection(&data_CS);//ɾ���ؼ�����ζ���
	return 0;
}

DWORD download(LPVOID lpParameter)
{
	//���ݿ����ӹؼ���
	const char * SERVER = MYSQL_SERVER.data() ;
	const char * USERNAME = MYSQL_USERNAME.data();
	const char * PASSWORD = MYSQL_PASSWORD.data();
	const char DATABASE[20] = "uav_message";
	const int PORT = 3306;
	MySQLInterface mysql;//�������ݿ����Ӷ���
	//�������ݿ�
	if (mysql.connectMySQL(SERVER, USERNAME, PASSWORD, DATABASE, PORT)) {
		while (1) {
			Sleep(100);
			EnterCriticalSection(&data_CS);//����ؼ������
			if (DATA_MESSAGES.empty()) {
				LeaveCriticalSection(&data_CS);
				continue;
			}
			//���ļ��ϴ�С
			int setLen = DATA_MESSAGES.size();
			LeaveCriticalSection(&data_CS);//�뿪�ؼ������

			while (1) {
				EnterCriticalSection(&data_CS);//����ؼ������
				char byte_data[70 * 1024];//ÿ�����Ŀռ����70K
				memcpy(byte_data, DATA_MESSAGES[0].val, 70 * 1024);//�����ĳ��е�һ������ȡ��
				LeaveCriticalSection(&data_CS);//�뿪�ؼ������
				string ackSql = "";
				DownMessage downMessage;
				downMessage.messageParse(byte_data);//��������
				char fileName[32];
				downMessage.getterFileName(fileName);
				char expandName[8];
				downMessage.getterExpandName(expandName);
				UINT32 taskNum = downMessage.getterTaskNum();//��ȡ������
				int size = 64 * 1024;
				char* data = new char[size];
				downMessage.getterData(data, size);//��ȡ����
				string taskNumFile = to_string(taskNum);
				//������ݿ��������ŵ����Ǳ��
				string sql = "select ���˻���� from �������� where ������ = " + taskNumFile + ";";
				vector<vector<string>> s;
				mysql.getDatafromDB(sql, s);
				string satelliteId = s[0][0];
				vector<vector<string>> disk;//����λ��
				MySQLInterface diskMysql;
				if (!diskMysql.connectMySQL(SERVER, USERNAME, PASSWORD, "disk", PORT)) {
					cout << "| ��������         | �������ݿ�ʧ��" << endl;
					cout << "| �������д�����Ϣ | " << diskMysql.errorNum << endl;
					delete data;
					break;
				}
				diskMysql.getDatafromDB("SELECT * FROM disk.����λ��;", disk);
				if (disk.size() == 0) {
					cout << "| ��������         | ����λ��δ֪" << endl;
					delete data;
					break;
				}
				string path = disk[0][1];
				path = path + "\\��������\\" + satelliteId;
				if (_access(path.c_str(), 0) == -1) {//����ļ��в�����
					_mkdir(path.c_str());//�򴴽�
					cout << "| �������б���·�� | " + path << endl;
				}
				//path = path + "\\" + taskNumFile;
				string file_path = path + "\\" + fileName + expandName;
				/*if (_access(path.c_str(), 0) == -1) {	
					_mkdir(path.c_str());				
					cout << "| �������б���·�� | " + path << endl;

				}*/
				//���ļ�
				ofstream ofs(file_path, ios::binary | ios::out | ios::app);
				ofs.write(data, size);
				ofs.close();
				delete data;
				EnterCriticalSection(&data_CS);//����ؼ������
				DATA_MESSAGES.erase(DATA_MESSAGES.begin(), DATA_MESSAGES.begin() + 1);//ɾ��Ԫ��
				LeaveCriticalSection(&data_CS);

				//�ж��Ƿ����ļ�β
				if (downMessage.getterEndFlag()) {
					//���ļ�βҪɾ����������
					//DATA_MESSAGES.erase(DATA_MESSAGES.begin(), DATA_MESSAGES.begin() + i + 1);
					cout << "| ��������         | �ļ�������ϣ����̣�"<< file_path << endl;
					string logSql = "insert into ϵͳ��־�� (ʱ��,����,�¼�����,�¼�˵��) values (now(),'��������ģ��',14010,'�������Ļ�:" + taskNumFile + " �յ����һ�����ݱ���');";
					mysql.writeDataToDB(logSql);
					break;//����ѭ��
				}
				else {
					int count = 0;
					//�ȴ��µ�����,���һ����
					while (count<60) {
						EnterCriticalSection(&data_CS);//����ؼ������
						if (DATA_MESSAGES.size() > 0) {
							LeaveCriticalSection(&data_CS);
							break;//�����ݵ�������ѭ��
						}
						LeaveCriticalSection(&data_CS);
						count++;
						Sleep(1000);
					}
					if (count == 60) {
						cout << "| ��������         | �ļ��ϴ����ݵȴ���ʱ" << endl;
						//ɾ����������
						DATA_MESSAGES.erase(DATA_MESSAGES.begin(), DATA_MESSAGES.begin() + 1);
						//ɾ���Ѿ���������
						remove(file_path.c_str());
						cout << "| ��������         | ����ļ��ѻ����ļ�" << endl;
						break;//����ѭ��
					}

				}
			}

		}
	}
	else {
		cout << "| ��������         | �������ݿ�ʧ��" << endl;
		cout << "| �������д�����Ϣ | " << mysql.errorNum << endl;
	}
	return 0;
}
