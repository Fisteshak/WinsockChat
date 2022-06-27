#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <vector>
#include "windows.h"

#pragma comment(lib, "Ws2_32.lib")

constexpr auto MAX_CONNECTIONS = 100;
constexpr int PORT = 10547;
constexpr int BUFLEN = 1024;
constexpr bool debug = false;


using std::cout, std::cin, std::endl;

void WSAInit()
{
	WSADATA wsaData;

	//инициализация интерфейса сокетов

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		cout << "Ошибка WSAStartup " << WSAGetLastError();
		exit(1);
	}
	else 
		if (debug) cout << "Winsock инициализирован" << endl;
}


SOCKET listenSock;

void socketInit(const int& addrFamily, const int& sockType)
{
	listenSock = socket(addrFamily, sockType, 0);

	if (listenSock == INVALID_SOCKET) {
		cout << "Ошибка инициализации сокета " << WSAGetLastError();
		closesocket(listenSock);
		WSACleanup();
		exit(1);
	}
	else 
		if (debug) cout << "Сокет сервера создан и инициализирован" << endl;

	in_addr serv_ip;


	if (inet_pton(addrFamily, "127.0.0.1", &serv_ip) <= 0) {
		cout << "Ошибка преобразования IP-адреса" << endl;
		exit(1);
	}

	int on = 1;

	if (setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0)
	{
		perror("setsockopt() failed");
		closesocket(listenSock);
		exit(1);
	}

	//перевести в неблок режим
	DWORD nonBlocking = 1;
	if (ioctlsocket(listenSock, FIONBIO, &nonBlocking) != 0)
	{
		cout << "Ошибка при переводе сокета в неблокирующий режим " << WSAGetLastError() << endl;
		exit(1);
	}

	//привязка к сокету адреса и порта
	sockaddr_in servInfo;

	ZeroMemory(&servInfo, sizeof(servInfo));    //обнулить

	servInfo.sin_family = AF_INET;
	servInfo.sin_port = htons(PORT);
	servInfo.sin_addr = serv_ip;

	int servInfoLen = sizeof(servInfo);



	if (bind(listenSock, (sockaddr*)&servInfo, servInfoLen) != 0) {
		cout << "Ошибка привязки сокета к порту и IP-адресу " << WSAGetLastError() << endl;
		closesocket(listenSock);
		WSACleanup();
		exit(1);
	}
	else if (debug) 
		cout << "Сокет успешно привязан" << endl;

	//перевести сокет в режим прослушивания


	if (listen(listenSock, SOMAXCONN) != 0) {
		cout << "Ошибка при переводе сокета в прослушивающий режим " << WSAGetLastError() << endl;
		closesocket(listenSock);
		WSACleanup();
		exit(1);
	}


}

enum ConsoleColor
{
	Black = 0,
	Blue = 1,
	Green = 2,
	Cyan = 3,
	Red = 4,
	Magenta = 5,
	Brown = 6,
	LightGray = 7,
	DarkGray = 8,
	LightBlue = 9,
	LightGreen = 10,
	LightCyan = 11,
	LightRed = 12,
	LightMagenta = 13,
	Yellow = 14,
	White = 15
};

void SetColor(ConsoleColor text, ConsoleColor background)
{
	HANDLE hConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsoleHandle, (WORD)((background << 4) | text));
}

int main()
{
	setlocale(LC_ALL, "Russian");
	SetConsoleCP(1251); // Ввод с консоли в кодировке 1251
	SetConsoleOutputCP(1251); // Вывод на консоль в кодировке 1251. Нужно только будет изменить шрифт консоли на Lucida Console или Consolas

	bool close_server = false;		//флаг завершения сервера
	bool compress_array = false;	//флаг сжатия массива соединений
	bool close_conn = false;		//флаг закрытия соединения

	int errStat;   //статус ошибки

	//инициализация WSA
	WSAInit();

	//создание и инициализация сокета
	socketInit(AF_INET, SOCK_STREAM);


	struct pollfd fds[MAX_CONNECTIONS];             //массив соединений
	int fdsNum = 1;									//количество соединений в fds

	memset(fds, 0, sizeof(fds));					//инициализировать fds

	fds[0].fd = listenSock;							//в 1й ячейке слушающий сокет
	fds[0].events = POLLIN;

	std::vector <std::string> usernames(MAX_CONNECTIONS);				//массив имен клиентов, в 0-й ячейке (сокет сервера) пусто


	do {
		errStat = WSAPoll(fds, fdsNum, -1);

		if (errStat == SOCKET_ERROR) {
			cout << "Ошибка WSAPoll() " << WSAGetLastError() << endl;
			close_server = true;
			break;
		}

		if (fds[0].revents != 0) {			//если в 0м сокете есть событие, то это входящее соединение
			while (true) {

				SOCKET newConn = accept(listenSock, NULL, NULL);	//принять соединение
				if (newConn == INVALID_SOCKET) {						//если accept возвращает EWOULDBLOCK, то соединений больше нет
					break;
				}
				fds[fdsNum].fd = newConn;						//добавить соединение в структуру fds
				fds[fdsNum].events = POLLIN;
				fdsNum++;
							
			}
			fds[0].revents = 0;
		}

		std::vector <char> buf(BUFLEN);  //буфер для входящих данных
			//просмотреть остальные соединения
			//если в них есть событие, то это входящее сообщение
		int i;
		int fdsNumCp = fdsNum;
		for (i = 1; i < fdsNum; i++)
			if (fds[i].revents != 0) {		//если есть событие
				fds[i].revents = 0;
				int dataLen = recv(fds[i].fd, buf.data(), BUFLEN, 0);  //получить данные				

				if (dataLen < 0) {					//если есть ошибка, то закрываем соединение
					if (WSAGetLastError() != WSAECONNRESET)		//WSAECONNRESET означает, что клиент отключился
						cout << "Ошибка при получении данных " << WSAGetLastError() << endl; 
					close_conn = true;					//закрываем соединение				
				}

				if (dataLen == 0) {		//если передали 0 байт, то клиент отключился
					close_conn = true;					//закрываем соединение				
				}

				if (close_conn) {			//закрываем соединение
					closesocket(fds[i].fd);
					SetColor(Green, Black);					
					cout << usernames[i] << " отключился." << endl;
					SetColor(White, Black);
					fds[i].fd = -1;
					compress_array = true;	//сжать массив соединений
					close_conn = false;
					continue;
				}

				//если у клиента есть имя, то это сообщение
				//иначе это имя

				std::string bufSend = buf.data();
				bufSend = usernames[i] + ": " + bufSend;

				if (usernames[i] != "") {

					cout << usernames[i] << ": " << buf.data() << endl;
					//отправить это сообщение остальным клиентам					
					for (auto j = 1; j < fdsNum; j++)
						if (i != j) {
							dataLen = send(fds[j].fd, bufSend.data(), BUFLEN, 0);
							if (dataLen < 0) {					//если есть ошибка, то закрываем соединение
								cout << "Ошибка при отправке данных " << WSAGetLastError() << endl;
								//close_conn = true;					//закрываем соединение				
							}

						}
				}
				else {
					usernames[i] = buf.data();
					SetColor(Green, Black);
					cout << usernames[i] << " подключился." << endl;
					SetColor(White, Black);
				}
			}

		//если мы закрыли соединение, то в массиве соединений остается пустое место
		//здесь мы его сжимаем

		if (compress_array) {
			for (i = 0; i < fdsNum; i++)
			{
				if (fds[i].fd == -1)
				{
					for (int j = i; j < fdsNum; j++)
					{
						fds[j].fd = fds[j + 1].fd;
						usernames[j] = usernames[j + 1];
					}
					i--;
					fdsNum--;
				}
			}
			compress_array = false;
		}

	} while (close_server == false);


	for (int i = 0; i < fdsNum; i++)
	{
		if (fds[i].fd >= 0)
			closesocket(fds[i].fd);
	}

	WSACleanup();

	return 0;



}