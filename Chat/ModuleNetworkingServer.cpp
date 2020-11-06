#include "ModuleNetworkingServer.h"




//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer public methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::start(int port)
{
	// TODO(jesus): TCP listen socket stuff

	// - Create the listenSocket
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
	{
		ELOG("[SERVER ERROR]: socket creation %d", WSAGetLastError());
		
	}

	// - Set address reuse
	int enable = 1;
	int iResult = setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(enable));	
	if (iResult == SOCKET_ERROR)
	{
		ELOG("[SERVER ERROR]: address reuse %d", WSAGetLastError());
		//ModuleNetworking::printWSErrorAndExit("[SETSOCKOPT]");
		return false;
	}

	// - Bind the socket to a local interface
	sockaddr_in localAddr;
	localAddr.sin_family = AF_INET; // IPv4
	localAddr.sin_port = htons(port); // Port
	localAddr.sin_addr.S_un.S_addr = INADDR_ANY; // Any local IP address
	iResult = bind(listenSocket, (sockaddr*)&localAddr, sizeof(localAddr));
	if (iResult == SOCKET_ERROR)
	{
		ELOG("[SERVER ERROR]: Bind socket to local interface %d", WSAGetLastError());
		
		return false;
	}

	// - Enter in listen mode
	int simultaneousConnections = 1;
	iResult = listen(listenSocket, simultaneousConnections);
	if (iResult == SOCKET_ERROR)
	{
		ELOG("[SERVER ERROR]: listen mode %d", WSAGetLastError());
		
		return false;
	}

	// - Add the listenSocket to the managed list of sockets using addSocket()
	addSocket(listenSocket);

	state = ServerState::Listening;

	return true;
}

bool ModuleNetworkingServer::isRunning() const
{
	return state != ServerState::Stopped;
}



//////////////////////////////////////////////////////////////////////
// Module virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::update()
{
	return true;
}

bool ModuleNetworkingServer::gui()
{
	if (state != ServerState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Server Window");

		Texture *tex = App->modResources->server;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("List of connected sockets:");

		for (auto &connectedSocket : connectedSockets)
		{
			ImGui::Separator();
			ImGui::Text("Socket ID: %d", connectedSocket.socket);
			ImGui::Text("Address: %d.%d.%d.%d:%d",
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b1,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b2,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b3,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b4,
				ntohs(connectedSocket.address.sin_port));
			ImGui::Text("Player name: %s", connectedSocket.playerName.c_str());
		}

		ImGui::End();
	}

	return true;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::isListenSocket(SOCKET socket) const
{
	return socket == listenSocket;
}

void ModuleNetworkingServer::onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress)
{
	// Add a new connected socket to the list
	ConnectedSocket connectedSocket;
	connectedSocket.socket = socket;
	connectedSocket.address = socketAddress;
	connectedSockets.push_back(connectedSocket);
}

void ModuleNetworkingServer::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{
	ClientMessage clientMessage;
	packet >> clientMessage;

	if (clientMessage == ClientMessage::Hello)
	{
		LOG("Hello message received from the connected client");

		std::string playerName;
		packet >> playerName;

		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.playerName == playerName)
			{
				//Sending Non-Welcome message to client
				OutputMemoryStream outPacket;
				outPacket << ServerMessage::PlayerNameUnavailable;

				if (ModuleNetworking::sendPacket(outPacket, socket))
				{
					LOG("Non-Welcome message send to connected client");
				}
				else
				{
					ELOG("[SERVER ERROR]: error sending Non-Welcome message to connected client");
				}

				return;
			}
		}

		// Set the player name of the corresponding connected socket proxy
		for (auto &connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				//connectedSocket.playerName = (const char *)packet.GetBufferPtr();
				connectedSocket.playerName = playerName;

				//Send Welcome to client
				OutputMemoryStream outPacket;
				outPacket << ServerMessage::Welcome;

				
				if (ModuleNetworking::sendPacket(outPacket, socket))
				{
					LOG("Welcome message send to connected client");
				}
				else
				{
					ELOG("[SERVER ERROR]: error sending Welcome message to connected client");
				}
			}
		}
	}
	else if (clientMessage == ClientMessage::Typewrite)
	{
		Message msg;
		packet >> msg.playerName;
		packet >> msg.message;

		OutputMemoryStream outPacket;
		outPacket << ServerMessage::Typewrite;
		outPacket << msg.playerName;
		outPacket << msg.message;

		for (auto& connectedSocket : connectedSockets)
		{
			if (ModuleNetworking::sendPacket(outPacket, connectedSocket.socket))
			{
				LOG("Typing message send to connected clients");
			}
			else
			{
				ELOG("[SERVER ERROR]: error sending Typing message to connected clients");

			}
		}

	}
}

void ModuleNetworkingServer::onSocketDisconnected(SOCKET socket)
{
	// Remove the connected socket from the list
	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{
		auto &connectedSocket = *it;
		if (connectedSocket.socket == socket)
		{
			connectedSockets.erase(it);
			break;
		}
	}
}

