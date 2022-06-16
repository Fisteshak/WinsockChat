#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

constexpr auto BUFF_SIZE = 100;

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
	SOCKET clSock = socket(AF_INET, SOCK_STREAM, 0);

	if (clSock == INVALID_SOCKET) {
		cout << "Ошибка инициализации сокета " << WSAGetLastError();
		closesocket(clSock);
		WSACleanup();
		return 1;
	}
	else cout << "Сокет клиента создан и инициализирован" << endl;

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

	errStat = connect(clSock, (sockaddr*)&servInfo, sizeof(servInfo));

	if (errStat != 0) {
		cout << "Ошибка привязки сокета к серверу " << WSAGetLastError() << endl;
		closesocket(clSock);
		WSACleanup();
		return 1;
	}
	else cout << "Сокет успешно присоединен к серверу" << endl;


	
	std::vector <char> servBuff(BUFF_SIZE), clientBuff(BUFF_SIZE);							// Buffers for sending and receiving data
	short packet_size = 0;												// The size of sending / receiving packet in bytes

	while (true) {

		cout << "Your (Client) message to Server: ";
		fgets(clientBuff.data(), clientBuff.size(), stdin);

		// Check whether client like to stop chatting 
		if (clientBuff[0] == 'x' && clientBuff[1] == 'x' && clientBuff[2] == 'x') {
			shutdown(clSock, SD_BOTH);
			closesocket(clSock);
			WSACleanup();
			return 0;
		}

		packet_size = send(clSock, clientBuff.data(), clientBuff.size(), 0);

		if (packet_size == SOCKET_ERROR) {
			cout << "Can't send message to Server. Error # " << WSAGetLastError() << endl;
			closesocket(clSock);
			WSACleanup();
			return 1;
		}

		packet_size = recv(clSock, servBuff.data(), servBuff.size(), 0);

		if (packet_size == SOCKET_ERROR) {
			cout << "Can't receive message from Server. Error # " << WSAGetLastError() << endl;
			closesocket(clSock);
			WSACleanup();
			return 1;
		}
		else
			cout << "Server message: " << servBuff.data() << endl;

	}

	closesocket(clSock);
	WSACleanup();

	return 0;
}

