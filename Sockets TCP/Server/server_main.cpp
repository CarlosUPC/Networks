#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#include "WinSock2.h"
#include "Ws2tcpip.h"

#include <iostream>
#include <cstdlib>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#define LISTEN_PORT 8888

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

	// Create socket
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
	if (s == INVALID_SOCKET)
	{
		printWSErrorAndExit("socket");
		return;
	}

	// Configure socket fro address reuse
	int enable = 1;
	iResult = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(enable));
	if (iResult == SOCKET_ERROR) {
		printWSErrorAndExit("setsockopt");
		return;
	}

	// Associate socket to adress object and bind it to local address
	sockaddr_in localAddr;
	localAddr.sin_family = AF_INET; // IPv4
	localAddr.sin_port = htons(port); // Port
	localAddr.sin_addr.S_un.S_addr = INADDR_ANY; // Any local IP address
	iResult = bind(s, (sockaddr*)&localAddr, sizeof(localAddr));
	if (iResult == SOCKET_ERROR) {
		printWSErrorAndExit("bind");
		return;
	}

	// Make it listen socket
	int simultaneousConnections = 1;
	iResult = listen(s, simultaneousConnections);
	if (iResult == SOCKET_ERROR) {
		printWSErrorAndExit("listen");
		return;
	}

	// Accept new incoming connection from a remote host
	sockaddr_in remoteAddr;
	int remoteAddrSize = sizeof(remoteAddr);
	SOCKET connectedSocket = accept(s, (sockaddr*)&remoteAddr, &remoteAddrSize);
	if (connectedSocket == INVALID_SOCKET) {
		printWSErrorAndExit("accept");
		return;
	}

	char pingStr[5];
	
	while (true) {
		
		iResult = recv(connectedSocket, pingStr, sizeof(pingStr), 0);
		if (iResult == SOCKET_ERROR) {
			printWSErrorAndExit("recv");
			break;
		}
		else if (iResult == 0) {
			break;
		}

		std::cout << pingStr << std::endl;

		
		std::string pongStr = "Pong"; //Testing Api with STL string

		iResult = send(connectedSocket, pongStr.c_str(), pongStr.length()+1, 0);
		if (iResult == SOCKET_ERROR) {
			printWSErrorAndExit("send");
			break;
		}
	}


	// Delete socket
	iResult = closesocket(s);
	std::cout << "server socket has closed " << std::endl;
	if (iResult == SOCKET_ERROR)
	{
		printWSErrorAndExit("closesocket");
		return;
	}
	// CleanUp Winsock
	iResult = WSACleanup();
	std::cout << "server winsock has closed " << std::endl;
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

	server(atoi(argv[1]));

	system("pause");
	return 0;
}