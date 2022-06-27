#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

constexpr int PORT = 10547;
constexpr int BUFLEN = 1024;
constexpr bool debug = false;

using std::cout, std::cin, std::endl;

SOCKET clSock;

void WSAInit()
{
	WSADATA wsaData;

		//инициализация интерфейса сокетов
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		cout << "Ошибка WSAStartup " << WSAGetLastError();
		exit(1);
	}
	else if (debug) 
		cout << "Winsock инициализирован" << endl;
}


void socketInit(const PCSTR& IP, const int& addrFamily, const int& sockType)
{

	//создание и инициализация сокета
	clSock = socket(addrFamily, sockType, 0);

	if (clSock == INVALID_SOCKET) {
		cout << "Ошибка инициализации сокета " << WSAGetLastError();
		closesocket(clSock);
		WSACleanup();
		exit(1);
	}
	else if (debug) 
		cout << "Сокет клиента создан и инициализирован" << endl;

	in_addr serv_ip;

	inet_pton(addrFamily, IP, &serv_ip);

	if (inet_pton(addrFamily, IP, &serv_ip) <= 0) {
		cout << "Ошибка преобразования IP-адреса" << endl;
		exit(1);
	}

	//привязка к сокету адреса и порта
	sockaddr_in servInfo;

	ZeroMemory(&servInfo, sizeof(servInfo));    //обнулить

	servInfo.sin_family = addrFamily;
	servInfo.sin_port = htons(PORT);
	servInfo.sin_addr = serv_ip;	

	if (connect(clSock, (sockaddr*)&servInfo, sizeof(servInfo)) != 0) {
		cout << "Ошибка привязки сокета к серверу " << WSAGetLastError() << endl;
		closesocket(clSock);
		WSACleanup();
		exit(1);
	}
	else if (debug) 
		cout << "Сокет успешно присоединен к серверу" << endl;
	
}

bool end_client = false; //надо ли завершать работу клиента

void recvThread()
{
	int packet_size;
	std::vector <char> bufRecv(BUFLEN);							// буфер для получения

	while (!end_client) {
		bufRecv.assign(BUFLEN, 0);
		packet_size = recv(clSock, bufRecv.data(), 1024, 0);

		if (packet_size == 0) {
			cout << "Сервер прекратил работу." << endl;
			
		}
		if (packet_size < 0) {
			if (WSAGetLastError() != WSAECONNRESET) 
				cout << "Ошибка при получении данных " << WSAGetLastError() << endl;
			else {
				cout << "Сервер прекратил работу." << endl;
				end_client = true;
			}
		}
		if (packet_size > 0) {
			cout << bufRecv.data() << endl;
		}

	}
}

int main()
{
	setlocale(LC_ALL, "Russian");
	SetConsoleCP(1251); // Ввод с консоли в кодировке 1251
	SetConsoleOutputCP(1251); // Вывод на консоль в кодировке 1251. Нужно только будет изменить шрифт консоли на Lucida Console или Consolas

				//ввод имени пользователя
	std::string username;
	cout << "Введите ваши имя: " << std::flush;
	while (username.size() == 0) getline(cin, username);


	WSAInit();			//инициализировать WSA

	socketInit("127.0.0.1", AF_INET, SOCK_STREAM);   //инициализация и привязка сокета (clSock) к серверу

	
	std::string bufSend;										// буфер для отправки
	short packet_size = 0;										// The size of sending / receiving packet in bytes

			//отправить имя серверу
			//первая посылка к серверу считается за имя
	packet_size = send(clSock, username.data(), username.size(), 0);

			//поток для получения данных
	std::thread recvThr(recvThread);

	while (!end_client) {
		
		while (bufSend.size() == 0) getline(cin, bufSend);					
			
		packet_size = send(clSock, bufSend.data(), bufSend.size(), 0);
		bufSend.clear();
		if (packet_size == SOCKET_ERROR) {
			cout << "Can't send message to Server. Error # " << WSAGetLastError() << endl;
			closesocket(clSock);
			WSACleanup();
			exit(1);
		}

	}

	recvThr.join();

	closesocket(clSock);
	WSACleanup();

	return 0;
}

