#include "Networks.h"
#include "ModuleNetworking.h"
#include <algorithm>

static uint8 NumModulesUsingWinsock = 0;



void ModuleNetworking::reportError(const char* inOperationDesc)
{
	LPVOID lpMsgBuf;
	DWORD errorNum = WSAGetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	ELOG("Error %s: %d- %s", inOperationDesc, errorNum, lpMsgBuf);
}

void ModuleNetworking::printWSErrorAndExit(const char* msg)
{
	wchar_t* s = NULL;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s, 0, NULL);
	fprintf(stderr, "%s: %S\n", msg, s);
	LocalFree(s);
}

void ModuleNetworking::disconnect()
{
	for (SOCKET socket : sockets)
	{
		shutdown(socket, 2);
		closesocket(socket);
	}

	sockets.clear();
}

bool ModuleNetworking::init()
{
	if (NumModulesUsingWinsock == 0)
	{
		NumModulesUsingWinsock++;

		WORD version = MAKEWORD(2, 2);
		WSADATA data;
		if (WSAStartup(version, &data) != 0)
		{
			reportError("ModuleNetworking::init() - WSAStartup");
			return false;
		}
	}

	return true;
}

bool ModuleNetworking::preUpdate()
{
	if (sockets.empty()) return true;

	// NOTE(jesus): You can use this temporary buffer to store data from recv()

	//const uint32 incomingDataBufferSize = Kilobytes(1);
	//byte incomingDataBuffer[incomingDataBufferSize];

	// TODO(jesus): select those sockets that have a read operation available
	fd_set socketSet;
	socketSet.fd_count = sockets.size();
	//memcpy(socketSet.fd_array, &sockets[0], sockets.size() * sizeof(SOCKET));
	std::copy(sockets.begin(), sockets.end(), socketSet.fd_array);

	TIMEVAL timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	int res = select(0, &socketSet, nullptr, nullptr, &timeout);
	if (res == SOCKET_ERROR) 
	{
		ELOG("[NETWORKING ERROR]: select read sockets %d", WSAGetLastError());
		//ModuleNetworking::printWSErrorAndExit("[NETWORKING ERROR]: Select read sockets");
	}

	// TODO(jesus): for those sockets selected, check wheter or not they are
	// a listen socket or a standard socket and perform the corresponding
	// operation (accept() an incoming connection or recv() incoming data,
	// respectively).
	// On accept() success, communicate the new connected socket to the
	// subclass (use the callback onSocketConnected()), and add the new
	// connected socket to the managed list of sockets.
	// On recv() success, communicate the incoming data received to the
	// subclass (use the callback onSocketReceivedData()).

	// TODO(jesus): handle disconnections. Remember that a socket has been
	// disconnected from its remote end either when recv() returned 0,
	// or when it generated some errors such as ECONNRESET.
	// Communicate detected disconnections to the subclass using the callback
	// onSocketDisconnected().

	// TODO(jesus): Finally, remove all disconnected sockets from the list
	// of managed sockets.


	for (int i = 0; i < socketSet.fd_count; ++i)
	{
		SOCKET s_tmp = socketSet.fd_array[i];

		if (isListenSocket(s_tmp))
		{
			sockaddr_in clientAddr;
			int clientAddrSize = sizeof(clientAddr);

			SOCKET connected = accept(s_tmp, (sockaddr*)&clientAddr, &clientAddrSize);

			if (connected == INVALID_SOCKET)
			{
				ELOG("[NETWORKING ERROR]: Error receiving client connection %d", WSAGetLastError());
				return false;
			}

			onSocketConnected(connected, clientAddr);
			sockets.push_back(connected);
		}
		else
		{
			InputMemoryStream packet;
			int result = recv(s_tmp, packet.GetBufferPtr(), packet.GetCapacity(), 0);
			//int result = recv(s_tmp, (char*)&incomingDataBuffer, incomingDataBufferSize, 0);

			if (result == SOCKET_ERROR) // errors generated from remote socket
			{
				ELOG("[NETWORKING ERROR]: Error receiving data from connected sockets %d", WSAGetLastError());
				onSocketDisconnected(s_tmp);
				for (auto it = sockets.begin(); it != sockets.end(); ++it)
				{
					if ((*it) == s_tmp)
					{
						sockets.erase(it);
						break;
					}
				}
				return false;
			}
			else if (result == INVALID_SOCKET) // remote socket was disconnected
			{
				ELOG("[NETWORKING ERROR]: Error receiving data from connected sockets %d", WSAGetLastError());
				onSocketDisconnected(s_tmp);
				for (auto it = sockets.begin(); it != sockets.end(); ++it)
				{
					if ((*it) == s_tmp)
					{
						sockets.erase(it);
						break;
					}
				}
				return false;
			}
			else
			{
				//onSocketReceivedData(s_tmp, incomingDataBuffer);
				packet.SetSize((uint32)result);
				onSocketReceivedData(s_tmp, packet);
			}
		}

	}
	

	return true;
}

bool ModuleNetworking::cleanUp()
{
	disconnect();

	NumModulesUsingWinsock--;
	if (NumModulesUsingWinsock == 0)
	{

		if (WSACleanup() != 0)
		{
			reportError("ModuleNetworking::cleanUp() - WSACleanup");
			return false;
		}
	}

	return true;
}

bool ModuleNetworking::sendPacket(const OutputMemoryStream& packet, SOCKET socket)
{
	int result = send(socket, packet.GetBufferPtr(), packet.GetSize(), 0);
	if (result == SOCKET_ERROR)
	{
		ModuleNetworking::reportError("send");
		return false;
	}
	return true;
}

void ModuleNetworking::addSocket(SOCKET socket)
{
	sockets.push_back(socket);
}
