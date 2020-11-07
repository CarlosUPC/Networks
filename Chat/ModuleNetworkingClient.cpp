#include "ModuleNetworkingClient.h"


bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName)
{
	playerName = pplayerName;

	// TODO(jesus): TCP connection stuff

	// - Create the socket
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		ELOG("[CLIENT ERROR]: socket creation %d", WSAGetLastError());
		
		return false;
	}

	// - Create the remote address object
	//sockaddr_in toAddr;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(serverPort);
	const char* toAddrStr = serverAddressStr;
	int iResult = inet_pton(AF_INET, toAddrStr, &serverAddress.sin_addr);
	if (iResult == SOCKET_ERROR)
	{
		ELOG("[CLIENT ERROR]: remote address Creation %d", WSAGetLastError());
		
		return false;
	}

	// - Connect to the remote address
	iResult = connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR)
	{
		ELOG("[CLIENT ERROR]: connect socket to romete adress %d", WSAGetLastError());
		
		return false;
	}

	// - Add the created socket to the managed list of sockets using addSocket()
	addSocket(clientSocket);

	// If everything was ok... change the state
	state = ClientState::Start;

	return true;
}

bool ModuleNetworkingClient::isRunning() const
{
	return state != ClientState::Stopped;
}

bool ModuleNetworkingClient::update()
{
	bool ret = true;
	if (state == ClientState::Start)
	{
		OutputMemoryStream packet;
		packet << ClientMessage::Hello;
		packet << playerName;

		if (ModuleNetworking::sendPacket(packet, clientSocket))
		{
			LOG("Hello message send to the server");
			state = ClientState::Logging;
		}
		else {
			ELOG("[CLIENT ERROR]: Error sending <playerName> & Hello message to server");
			disconnect();
			state = ClientState::Stopped;
		}


		// TODO(jesus): Send the player name to the server
		/*int iResult = send(clientSocket, playerName.c_str(), playerName.size() + 1, 0);
		if (iResult == SOCKET_ERROR)
		{
			ELOG("[CLIENT ERROR]: Error sending playername to server %d", WSAGetLastError());
			state = ClientState::Stopped;
			ret = false;
		}
		else {
			state = ClientState::Logging;
		}*/
	}

	return ret;
}

bool ModuleNetworkingClient::gui()
{
	if (state != ClientState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Client Window");

		Texture *tex = App->modResources->client;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("%s connected to the server...", playerName.c_str());

		for (Message msg : messages)
		{
			if (msg.notify)
				ImGui::TextColored({ 1,1,0,1 }, "\"%s\".", msg.message.data());
			else
			{
				if(msg.whisper)
					ImGui::TextColored({ 0.55,0.55,0.55,1 }, "%s: %s", msg.playerName.data(), msg.message.data());
				else
					//ImGui::TextColored({msg.color[0],msg.color[0],msg.color[0],1 }, "%s: %s", msg.playerName.data(), msg.message.data());
				ImGui::Text("%s: %s", msg.playerName.data(), msg.message.data());

			}
		}

		char message[1024] = "";
		ImVec2 windowPos = ImGui::GetWindowPos();
		ImGui::SetCursorScreenPos({ ImGui::GetCursorScreenPos().x, windowPos.y + ImGui::GetWindowHeight() - 30 });
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Message:"); ImGui::SameLine();
		if(ImGui::InputText("##MessageInput", message, 1024, ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue))
		{
			Message msg;
			msg.message = message;
			msg.playerName = playerName;

			OutputMemoryStream packet;
			packet << ClientMessage::Typewrite;
			packet << msg.playerName;
			packet << msg.message;
			
			sendPacket(packet, clientSocket);

			//To keep the input text focused after sending the message
			ImGui::SetKeyboardFocusHere(-1);
		}

		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{
	//state = ClientState::Stopped;
	
	ServerMessage serverMessage;
	packet >> serverMessage;

	if (state == ClientState::Start)
	{
		if (serverMessage == ServerMessage::Welcome)
		{
			LOG("Welcome received from the server");
		}
		else if(serverMessage == ServerMessage::PlayerNameUnavailable)
		{
			LOG("Non-Welcome received from the server");
			state = ClientState::Stopped;
		}
		else
		{
			LOG("data not received from the server");
			state = ClientState::Stopped;
			disconnect();
		}

	}
	else if (state == ClientState::Logging)
	{
		if (serverMessage == ServerMessage::Typewrite)
		{
			//TOD: Store all messages to display them with imgui
			Message msg;
			packet >> msg.playerName;
			packet >> msg.message;
			packet >> msg.whisper;
			//packet >> msg.color;

			messages.push_back(msg);
		}
		else if (serverMessage == ServerMessage::Notification)
		{
			Message msg;
			packet >> msg.message;
			msg.notify = true;

			messages.push_back(msg);
		}
		else if (serverMessage == ServerMessage::Kick)
		{
			messages.clear();
			state = ClientState::Stopped;
			disconnect();
		}
		else if (serverMessage == ServerMessage::ChangeName)
		{
			packet >> playerName;

			Message msg;
			packet >> msg.message;
			msg.notify = true;

			messages.push_back(msg);
			
		}
		else if (serverMessage == ServerMessage::ChangeColour)
		{
			
		}
		else if (serverMessage == ServerMessage::Clear)
		{
			messages.clear();
		}
		
		
	}
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
}

