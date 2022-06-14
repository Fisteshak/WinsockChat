#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

using std::cout, std::cin, std::endl;

int main()
{
	setlocale(LC_ALL, "Russian");

	WSADATA wsaData;
	int errSt;   //статус ошибки


		//инициализация интерфейса сокетов
	errSt = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (errSt != 0) {
		cout << "Ошибка WSAStartup " << WSAGetLastError();
		return 1;
	}
	else cout << "Winsock инициализирован" << endl;

		//создание и инициализация сокета
	SOCKET servSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (servSocket == INVALID_SOCKET) {
		cout << "Ошибка инициализации сокета " << WSAGetLastError();
		closesocket(servSocket);
		WSACleanup();
		return 1;
	}
	else cout << "Сокет сервера создан и инициализирован" << endl;

	
	in_addr serv_ip;
	

	errSt = inet_pton(AF_INET, "127.0.0.1", &serv_ip);

	if (errSt <= 0) {
		cout << "Ошибка преобразования IP-адреса" << endl;
		return 1;
	}

		//привзяка к сокету адреса и порта

	sockaddr_in servInfo;
	ZeroMemory(&servInfo, sizeof(servInfo));

	servInfo.sin_family = AF_INET;
	servInfo.sin_port = htons(1234);
	servInfo.sin_addr = serv_ip;

	errSt = bind(servSocket, (sockaddr*)&servInfo, sizeof(servInfo));
	if (errSt != 0) {
		cout << "Ошибка привязки сокета к порту и IP-адресу " << WSAGetLastError() << endl;
		closesocket(servSocket);
		WSACleanup();
		return 1;
	}
	else cout << "Сокет успешно привязан" << endl;

	




}

