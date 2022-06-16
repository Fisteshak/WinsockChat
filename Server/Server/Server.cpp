#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include "windows.h"

#pragma comment(lib, "Ws2_32.lib")

using std::cout, std::cin, std::endl;

int main()
{
	setlocale(LC_ALL, "Russian");

	WSADATA wsaData;
	int errStat;   //статус ошибки


		//инициализация интерфейса сокетов
	errStat = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (errStat != 0) {
		cout << "Ошибка WSAStartup " << WSAGetLastError();
		return 1;
	}
	else cout << "Winsock инициализирован" << endl;

	//создание и инициализация сокета
	SOCKET servSock = socket(AF_INET, SOCK_STREAM, 0);

	if (servSock == INVALID_SOCKET) {
		cout << "Ошибка инициализации сокета " << WSAGetLastError();
		closesocket(servSock);
		WSACleanup();
		return 1;
	}
	else cout << "Сокет сервера создан и инициализирован" << endl;

	in_addr serv_ip;

	errStat = inet_pton(AF_INET, "127.0.0.1", &serv_ip);

	if (errStat <= 0) {
		cout << "Ошибка преобразования IP-адреса" << endl;
		return 1;
	}

	//привязка к сокету адреса и порта
	sockaddr_in servInfo;

	ZeroMemory(&servInfo, sizeof(servInfo));    //обнулить

	servInfo.sin_family = AF_INET;
	servInfo.sin_port = htons(10547);
	servInfo.sin_addr = serv_ip;

	errStat = bind(servSock, (sockaddr*)&servInfo, sizeof(servInfo));
	if (errStat != 0) {
		cout << "Ошибка привязки сокета к порту и IP-адресу " << WSAGetLastError() << endl;
		closesocket(servSock);
		WSACleanup();
		return 1;
	}
	else cout << "Сокет успешно привязан" << endl;


	//перевод сокета в неблокирующий режим
	// ? как работать с неблок
/*DWORD nonBlocking = 1;
if (ioctlsocket(servSock, FIONBIO, &nonBlocking) != 0)
{
	printf("failed to set non-blocking socket\n");
	return false;
}*/

//прослушивание сокета


	errStat = listen(servSock, SOMAXCONN);

	if (errStat != 0) {
		cout << "Ошибка при прослушивании соединений " << WSAGetLastError() << endl;
		closesocket(servSock);
		WSACleanup();
		return 1;
	}
	else cout << "Слушаю..." << endl;

	//принять соединение

	sockaddr_in clientInfo;


	ZeroMemory(&clientInfo, sizeof(clientInfo));

	int clientInfo_size = sizeof(clientInfo);

	SOCKET clConn;				//сокет клиента, который подключается к нам

	clConn = accept(servSock, (sockaddr*)&clientInfo, &clientInfo_size);

	if (clConn == INVALID_SOCKET) {
		cout << "Клиент был обнаружен, но произошла ошибка подсоединения " << WSAGetLastError() << endl;
		closesocket(servSock);
		closesocket(clConn);
		WSACleanup();
		return 1;
	}
	else
		cout << "Клиент успешно подключён" << endl;

	std::vector <char> clBuff(100);
	
	constexpr int BUFF_SIZE = 100;

	std::vector <char> servBuff(BUFF_SIZE), clientBuff(BUFF_SIZE);							// Creation of buffers for sending and receiving data
	short packet_size = 0;												// The size of sending / receiving packet in bytes

	while (true) {
		packet_size = recv(clConn, servBuff.data(), servBuff.size(), 0);					// Receiving packet from client. Program is waiting (system pause) until receive
		cout << "Client's message: " << servBuff.data() << endl;

		cout << "Your (host) message: ";
		fgets(clientBuff.data(), clientBuff.size(), stdin);

		// Check whether server would like to stop chatting 
		if (clientBuff[0] == 'x' && clientBuff[1] == 'x' && clientBuff[2] == 'x') {
			shutdown(clConn, SD_BOTH);
			closesocket(servSock);
			closesocket(clConn);
			WSACleanup();
			return 0;
		}

		packet_size = send(clConn, clientBuff.data(), clientBuff.size(), 0);

		if (packet_size == SOCKET_ERROR) {
			cout << "Can't send message to Client. Error # " << WSAGetLastError() << endl;
			closesocket(servSock);
			closesocket(clConn);
			WSACleanup();
			return 1;
		}

	}

	closesocket(servSock);
	closesocket(clConn);
	WSACleanup();

	return 0;



}