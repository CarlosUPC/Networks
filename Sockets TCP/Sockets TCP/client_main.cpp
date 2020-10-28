#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"

#include <iostream>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_ADDRESS "127.0.0.1"

#define SERVER_PORT 8888

void printWSErrorAndExit(const char* msg)
{
	wchar_t* s = NULL;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s, 0, NULL);
	fprintf(stderr, "%s: %S\n", msg, s);
	LocalFree(s);
	system("pause");
	exit(-1);
}

void client(const char* serverAddrStr, int port)
{
	// Init Winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult == SOCKET_ERROR)
	{
		// Log and handle error
		printWSErrorAndExit("WSAStartup");
		return;
	}

	// Create socket
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
	if (s == INVALID_SOCKET)
	{
		printWSErrorAndExit("socket");
		return;
	}

	// Associate socket to adress object
	sockaddr_in toAddr;
	toAddr.sin_family = AF_INET; // IPv4
	toAddr.sin_port = htons(port); // Port
	const char* toAddrStr = serverAddrStr; // Not so remote… :-P
	inet_pton(AF_INET, toAddrStr, &toAddr.sin_addr);

	// Connect socket to server
	iResult = connect(s, (sockaddr*)&toAddr, sizeof(toAddr));
	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("connect");
		return;
	}

	
	char pongStr[5]; // Testing Api with stack allocation
	//char* pong = new char[5]; // Testing Api with heap allocation

	for (int i = 0; i < 5; ++i)
	{
		
		// Send data to server
		iResult = send(s, "Ping", strlen("Ping")+1, 0); // Testing Api with string + /0
		
		if (iResult == SOCKET_ERROR)
		{
			printWSErrorAndExit("send");
			break;
		}

		// Receive data from server
		
		iResult = recv(s, pongStr,  sizeof(pongStr), 0);

		if (iResult == SOCKET_ERROR)
		{
			printWSErrorAndExit("recv");
			break;
		}

		std::cout << pongStr << std::endl;
		Sleep(500);
	}


	// Delete socket
	iResult = closesocket(s);
	std::cout << "remote client socket has closed " << std::endl;
	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("closesocket");
		return;
	}
	// CleanUp Winsock
	iResult = WSACleanup();
	std::cout << "remote client winsock has closed " << std::endl;
	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("WSACleanup");
		return;
	}
}


int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cout << "[ERROR]: No Input Arguments (IP Address & Port) entered" << std::endl;
		return 0;
	}

	const char* server_addr = argv[1];
	int server_port = atoi(argv[2]);

	client(server_addr, server_port);
	
	system("pause");
	return 0;
}