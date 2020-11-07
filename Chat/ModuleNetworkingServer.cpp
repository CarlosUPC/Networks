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

				//Notify everyone a new client has been connected
				OutputMemoryStream outPacketConnection;
				outPacketConnection << ServerMessage::Notification;
				outPacketConnection << playerName + " joined";

				for (auto& s : connectedSockets)
				{
					if (s.socket == socket)
						continue;

					if (ModuleNetworking::sendPacket(outPacketConnection, s.socket))
					{
						LOG("Welcome message send to connected client");
					}
					else
					{
						ELOG("[SERVER ERROR]: error sending Notifiaction message to connected client");
					}
				}

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
			//else
			//{
			//	//Send JOINED Message to all the other clients
			//}
		}
	}
	else if (clientMessage == ClientMessage::Typewrite)
	{
		Message msg;
		packet >> msg.playerName;
		packet >> msg.message;

		if (msg.message[0] != '/')
		{
			OutputMemoryStream outPacket;
			outPacket << ServerMessage::Typewrite;
			outPacket << msg.playerName;
			outPacket << msg.message;
			outPacket << msg.whisper;
			//outPacket << msg.color;

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
		else
		{	
			// ------- SPECIAL COMMANDS ------- //

			// - Help
			if (msg.message == "/help")
			{
				OutputMemoryStream outPacket;
				outPacket << ServerMessage::Notification;
				//std::string help_text("Available Commands:\n/help: to list all available commands.\n/list: to list all users in the chat room.\n/kick: to ban some other user from the chat.\n/whisper: to send a message only to one user.\n/change_name: to change your username.");
				outPacket << "****************** Commands list *****************\n"
				"/help\n"
				"/list\n"
				"/kick [username]\n"
				"/whisper [username] [message]\n"
					"/change_name [username]\n"
					"/clear\n"
					"/change_color [r] [g] [b]";

				if (ModuleNetworking::sendPacket(outPacket, socket))
				{
					LOG("Typing message send to connected clients");
				}
				else
				{
					ELOG("[SERVER ERROR]: error sending Typing message to connected clients");

				}
			}
			else if (msg.message == "/list")
			{
				OutputMemoryStream outPacket;
				outPacket << ServerMessage::Notification;

				std::string message = "Connected Users:";
				for (auto& connectedSocket : connectedSockets)
				{
					message += "\n- " + connectedSocket.playerName;
				}
				outPacket << message;

				if (ModuleNetworking::sendPacket(outPacket, socket))
				{
					LOG("[COMMAND]: </LIST> message send to connected clients");
				}
				else
				{
					ELOG("[SERVER ERROR]: error sending </LIST> message to connected clients");

				}
				
			}
			else if (msg.message.find("/kick") != std::string::npos)
			{
				std::string playerName = msg.message.substr(msg.message.find("/kick") + 6);

				for (auto& connectedSocket : connectedSockets)
				{
					if (connectedSocket.playerName == playerName)
					{
						OutputMemoryStream outPacket;
						outPacket << ServerMessage::Kick;

						if (ModuleNetworking::sendPacket(outPacket, connectedSocket.socket))
						{
							LOG("[COMMAND]:</KICK> message send to connected clients");
						}
						else
						{
							ELOG("[SERVER ERROR]: error sending </KICK> message to connected clients");

						}

						onSocketDisconnected(connectedSocket.socket);
					}
				}
			}
			else if (msg.message.find("/whisper") != std::string::npos)
			{
				std::string nameAndMessage = msg.message.substr(msg.message.find("/whisper") + 9);
				int nameEnd = nameAndMessage.find(" ");
				if (nameEnd != std::string::npos)
				{
					std::string name = nameAndMessage.substr(0, nameEnd);
					std::string message = nameAndMessage.substr(nameEnd + 1);

					bool found = false;
					for (auto& connectedSocket : connectedSockets)
					{
						if (connectedSocket.playerName == name)
						{
							found = true;

							OutputMemoryStream outPacket;
							outPacket << ServerMessage::Typewrite;
							outPacket << msg.playerName;
							outPacket << message;
							outPacket << true;

							//Sending message to whispered user
							if (ModuleNetworking::sendPacket(outPacket, connectedSocket.socket))
							{
								LOG("</Wisper> message send to connected clients");
							}
							else
							{
								ELOG("[SERVER ERROR]: error sending </Wisper> message to connected clients");

							}

							//Sending message to whisperer user
							if (connectedSocket.socket != socket)
							{
								if (ModuleNetworking::sendPacket(outPacket, socket))
								{
									LOG("</Wisper> message send to connected clients");
								}
								else
								{
									ELOG("[SERVER ERROR]: error sending </Wisper> message to connected clients");

								}
							}
						}
					}
					if (!found)
					{
						OutputMemoryStream outPacket;
						outPacket << ServerMessage::Notification;
						outPacket << "USER NOT FOUND";

						if (ModuleNetworking::sendPacket(outPacket, socket))
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
			else if (msg.message.find("/change_name") != std::string::npos)
			{
				std::string newPlayerName = msg.message.substr(msg.message.find("/change_name") + 13);
				
				for (auto& connectedSocket : connectedSockets)
				{
					if (connectedSocket.socket == socket)
					{
						connectedSocket.playerName = newPlayerName;
						
						OutputMemoryStream outPacket;
						outPacket << ServerMessage::ChangeName;
						outPacket << newPlayerName;
						outPacket << "changed name to " + newPlayerName;

						if (ModuleNetworking::sendPacket(outPacket, connectedSocket.socket))
						{
							LOG("[COMMAND]:</CHANGE_NAME> message send to connected clients");
						}
						else
						{
							ELOG("[SERVER ERROR]: error sending </CHANGE_NAME> message to connected clients");

						}

			
					}

				}
			}
			else if (msg.message.find("/change_color") != std::string::npos)
			{
			//TODO
				/*"/change_color 1.0 0.0 0.0";
				std::string r = "1.0";
				std::string g = "0.0";
				std::string b = "0.0";

				double r = 1.0;
				double g = 0.0;
				double b = 0.0;

				OutputMemoryStream outPacket;
				outPacket << ServerMessage::ChangeColorName;
				outPacket << r << g << b;
			

				if (ModuleNetworking::sendPacket(outPacket, socket))
				{
					LOG("[COMMAND]:</CHANGE_NAME> message send to connected clients");
				}
				else
				{
					ELOG("[SERVER ERROR]: error sending </CHANGE_NAME> message to connected clients");

				}*/

			}
			else if (msg.message == "/clear")
			{
				OutputMemoryStream outPacket;
				outPacket << ServerMessage::Clear;
				//outPacket << "console cleaned";

				if (ModuleNetworking::sendPacket(outPacket, socket))
				{
					LOG("[COMMAND]:</CLEAR> message send to connected clients");
				}
				else
				{
					ELOG("[SERVER ERROR]: error sending </CLEAR> message to connected clients");

				}
			}
			// - No available command
			else 
			{
				OutputMemoryStream outPacket;
				outPacket << ServerMessage::Notification;
				outPacket << msg.message + " is not an available command";

				if (ModuleNetworking::sendPacket(outPacket, socket))
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
}

void ModuleNetworkingServer::onSocketDisconnected(SOCKET socket)
{
	// Remove the connected socket from the list
	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{
		auto &connectedSocket = *it;
		if (connectedSocket.socket == socket)
		{
			//Notify everyone client has been disconnected
			OutputMemoryStream outPacketConnection;
			outPacketConnection << ServerMessage::Notification;
			outPacketConnection << connectedSocket.playerName + " left";

			for (auto& s : connectedSockets)
			{
				if (s.socket == socket)
					continue;

				if (ModuleNetworking::sendPacket(outPacketConnection, s.socket))
				{
					LOG("Welcome message send to connected client");
				}
				else
				{
					ELOG("[SERVER ERROR]: error sending Notifiaction message to connected client");
				}
			}

			connectedSockets.erase(it);
			return;
		}
	}
}

