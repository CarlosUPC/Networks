#include "ModuleNetworkingClient.h"


bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName)
{
	playerName = pplayerName;

	// TODO(jesus): TCP connection stuff

	// - Create the socket
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (clientSocket == INVALID_SOCKET)
	{
		ModuleNetworking::printWSErrorAndExit("[SOCKET]");
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
		ModuleNetworking::printWSErrorAndExit("[ADDRESS]");
		return false;
	}

	// - Connect to the remote address
	iResult = connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR)
	{
		ModuleNetworking::printWSErrorAndExit("[CONNECT]");
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
		// TODO(jesus): Send the player name to the server
		int iResult = send(clientSocket, playerName.c_str(), playerName.size() + 1, 0);
		if (iResult == SOCKET_ERROR)
		{
			ModuleNetworking::printWSErrorAndExit("[CLIENT ERROR]: Error sending playername to server");
			state = ClientState::Stopped;
			ret = false;
		}
		else {
			state = ClientState::Logging;
		}
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

		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, byte * data)
{
	state = ClientState::Stopped;
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
}

