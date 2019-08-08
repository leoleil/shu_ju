// Server.cpp : Defines the entry point for the console application.  
//  
#include <iostream> 
#include <thread> //thread ͷ�ļ�,ʵ�����й��̵߳���
#include "winsock2.h"  
#include "MessageParser.h"
#include "MySQLInterface.h"
#include "updata.h"
#include "downdata.h"
using namespace std;

vector <message_byte> MESSAGE_VECTOR;//ȫ�ֱ��ĳ�
CRITICAL_SECTION g_CS;//ȫ�ֹؼ�����ζ���
int STOP = 0;//�Ƿ�ֹͣsocket��������
string MYSQL_SERVER = "";//���ӵ����ݿ�ip
string MYSQL_USERNAME = "";
string MYSQL_PASSWORD = "";

#define IDR_PAUSE 12
#define IDR_ABOUT 13
#define IDR_HIDE 14
#define IDR_SHOW 15
LPCTSTR szAppClassName = TEXT("�������Ļ��������");
LPCTSTR szAppWindowName = TEXT("�������Ļ��������");
HMENU hmenu;//�˵����
HWND hwnd_head = GetForegroundWindow();//ʹhwnd������ǰ�˵Ĵ���

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	NOTIFYICONDATA nid;
	UINT WM_TASKBARCREATED;
	POINT pt;//���ڽ����������
	int xx;//���ڽ��ղ˵�ѡ���ֵ

		   // ��Ҫ�޸�TaskbarCreated������ϵͳ�������Զ������Ϣ
	WM_TASKBARCREATED = RegisterWindowMessage(TEXT("TaskbarCreated"));
	switch (message)
	{
	case WM_CREATE://���ڴ���ʱ�����Ϣ.
		nid.cbSize = sizeof(nid);
		nid.hWnd = hwnd;
		nid.uID = 0;
		nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nid.uCallbackMessage = WM_USER;
		nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		lstrcpy(nid.szTip, szAppClassName);
		Shell_NotifyIcon(NIM_ADD, &nid);
		hmenu = CreatePopupMenu();//���ɲ˵�
								  //AppendMenu(hmenu, MF_STRING, IDR_SHOW, "��ʾ����");//Ϊ�˵��������ѡ��
		AppendMenu(hmenu, MF_STRING, IDR_HIDE, "���ش���");
		AppendMenu(hmenu, MF_STRING, IDR_PAUSE, "�˳�����");//Ϊ�˵��������ѡ��
		AppendMenu(hmenu, MF_STRING, IDR_ABOUT, "����");
		break;
	case WM_USER://����ʹ�øó���ʱ�����Ϣ.
		if (lParam == WM_LBUTTONDOWN) {
			//MessageBox(hwnd, TEXT("Win32 API ʵ��ϵͳ���̳���,˫�����̿����˳�!"), szAppClassName, MB_OK);
			ShowWindow(hwnd_head, SW_SHOW);
		}

		if (lParam == WM_LBUTTONDBLCLK)//˫�����̵���Ϣ,�˳�.
			SendMessage(hwnd, WM_CLOSE, wParam, lParam);
		if (lParam == WM_RBUTTONDOWN)
		{
			GetCursorPos(&pt);//ȡ�������
			::SetForegroundWindow(hwnd);//����ڲ˵��ⵥ������˵�����ʧ������
										//EnableMenuItem(hmenu, IDR_PAUSE, MF_GRAYED);//�ò˵��е�ĳһ����
			xx = TrackPopupMenu(hmenu, TPM_RETURNCMD, pt.x, pt.y, NULL, hwnd, NULL);//��ʾ�˵�����ȡѡ��ID
			if (xx == IDR_HIDE) {
				ShowWindow(hwnd_head, SW_HIDE);
			}

			if (xx == IDR_PAUSE) {
				//MessageBox(hwnd, TEXT("111"), szAppClassName, MB_OK); 
				SendMessage(hwnd, WM_CLOSE, wParam, lParam);
			}
			if (xx == IDR_ABOUT) MessageBox(hwnd, TEXT("ͨ�����Ļ�����"), szAppClassName, MB_OK);
			if (xx == 0) PostMessage(hwnd, WM_LBUTTONDOWN, NULL, NULL);
			//MessageBox(hwnd, TEXT("�Ҽ�"), szAppName, MB_OK);
		}
		break;
	case WM_DESTROY://��������ʱ�����Ϣ.
		Shell_NotifyIcon(NIM_DELETE, &nid);
		PostQuitMessage(0);
		break;
	default:
		/*
		* ��ֹ��Explorer.exe �����Ժ󣬳�����ϵͳϵͳ�����е�ͼ�����ʧ
		*
		* ԭ��Explorer.exe �����������ؽ�ϵͳ����������ϵͳ������������ʱ�����ϵͳ������
		* ע�����TaskbarCreated ��Ϣ�Ķ������ڷ���һ����Ϣ������ֻ��Ҫ��׽�����Ϣ�����ؽ�ϵ
		* ͳ���̵�ͼ�꼴�ɡ�
		*/
		if (message == WM_TASKBARCREATED)
			SendMessage(hwnd, WM_CREATE, wParam, lParam);
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}


int main(int argc, char* argv[])
{
	HWND hwnd;
	MSG msg;
	WNDCLASS wndclass;
	ifstream is("config.txt", ios::in);
	if (!is.is_open()) {
		cout << "| ���ݿ�����       | ";
		cout << " �޷��������ļ�������config.txt�Ƿ����ã�������ϵͳ" << endl;
		//�������ɹ��ͷ���Դ
		system("pause");
		return 0;
	}
	getline(is, MYSQL_SERVER);
	getline(is, MYSQL_USERNAME);
	getline(is, MYSQL_PASSWORD);
	is.close();

	InitializeCriticalSection(&g_CS);//��ʼ���ؼ�����ζ���
    //�����߳�
	

	//HANDLE hThread2;//��������
	//hThread2 = CreateThread(NULL, 0, updata, NULL, 0, NULL);
	//CloseHandle(hThread2);
	HANDLE hThread3;//���������߳�
	hThread3 = CreateThread(NULL, 0, download_rec, NULL, 0, NULL);
	CloseHandle(hThread3);

	HWND handle = FindWindow(NULL, szAppWindowName);
	if (handle != NULL)
	{
		MessageBox(NULL, TEXT("Application is already running"), szAppClassName, MB_ICONERROR);
		return 0;
	}

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = NULL;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppClassName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"), szAppClassName, MB_ICONERROR);
		return 0;
	}
	ShowWindow(hwnd_head, SW_HIDE);
	// �˴�ʹ��WS_EX_TOOLWINDOW ������������ʾ���������ϵĴ��ڳ���ť
	hwnd = CreateWindowEx(WS_EX_TOOLWINDOW,
		szAppClassName, szAppWindowName,
		WS_POPUP,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL, NULL, NULL, NULL);

	ShowWindow(hwnd, 0);
	UpdateWindow(hwnd);

	//��Ϣѭ��
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	//while (1) {
	//	if (STOP == 1)break;
	//}

	//Sleep(4000);//���̺߳�����Ĭ4��
	//DeleteCriticalSection(&g_CS);//ɾ���ؼ�����ζ���
	//system("pause");
	return 0;
}