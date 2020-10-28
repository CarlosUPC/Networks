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


void server(int port)
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


	// Create socket (IPv4)
	SOCKET s = socket(AF_INET, SOCK_DGRAM, 0); //UDP socket
	if (s == INVALID_SOCKET)
	{
		printWSErrorAndExit("socket");
		return;
	}


	// Force Adress reuse
	int enable = 1;
	iResult = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(enable));	
	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("setsockopt");
		return;
	}


	// Associate socket with and adress object and bind it to network interface to recieve incoming data
	sockaddr_in localAddr;
	localAddr.sin_family = AF_INET; // IPv4
	localAddr.sin_port = htons(port); // Port
	localAddr.sin_addr.S_un.S_addr = INADDR_ANY; // Any local IP address
	iResult = bind(s, (sockaddr*)&localAddr, sizeof(localAddr));
	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("bind");
		return;
	}

	
	char pingStr[5];
	
	while (true)
	{
		// Recieve data from client (ping)
		sockaddr_in fromAddr;
		int fromSize = sizeof(fromAddr);

		iResult = recvfrom(s, pingStr, sizeof(pingStr), 0, (sockaddr*)&fromAddr, &fromSize);
		if (iResult == SOCKET_ERROR)
		{
			printWSErrorAndExit("recvfrom");
			break;
		}
		
		std::cout << pingStr << std::endl;
		
		std::string pongStr = "Pong"; //Testing Api with STL string

		// Send data to client (pong)
		iResult = sendto(s, pongStr.c_str(), pongStr.length() + 1, 0, (sockaddr*)&fromAddr, fromSize);
		if (iResult == SOCKET_ERROR)
		{
			printWSErrorAndExit("sendto");
			break;
		}

	}


	// Close socket
	iResult = closesocket(s);
	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("closesocket");
		return;
	}

	// CleanUp Winsock
	iResult = WSACleanup();
	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("WSACleanup");
		return;
	}

}


int main(int argc, char* argv[])
{
 	
	if (argc != 2)
	{
		std::cout << "[ERROR]: No Input Arguments (Server Port) entered" << std::endl;
		return 0;
	}
	int server_port = atoi(argv[1]);
	server(server_port);

	system("pause");
	return 0;
}