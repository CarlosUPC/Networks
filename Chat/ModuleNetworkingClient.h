#pragma once

#include "ModuleNetworking.h"


struct Color
{
	double red = 0.0;
	double green = 0.0;
	double blue = 0.0;

};

struct Message
{
	std::string playerName;
	Color color = { (0.0), (0.0), (0.0) };
	std::string message;
	bool notify = false;
	bool whisper = false;


	

	//TODO: declare color variable (struct maybe)
	
};




class ModuleNetworkingClient : public ModuleNetworking
{
public:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingClient public methods
	//////////////////////////////////////////////////////////////////////

	bool start(const char *serverAddress, int serverPort, const char *playerName);

	bool isRunning() const;



private:

	//////////////////////////////////////////////////////////////////////
	// Module virtual methods
	//////////////////////////////////////////////////////////////////////

	bool update() override;

	bool gui() override;



	//////////////////////////////////////////////////////////////////////
	// ModuleNetworking virtual methods
	//////////////////////////////////////////////////////////////////////

	void onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet) override;

	void onSocketDisconnected(SOCKET socket) override;



	//////////////////////////////////////////////////////////////////////
	// Client state
	//////////////////////////////////////////////////////////////////////

	enum class ClientState
	{
		Stopped,
		Start,
		Logging
	};

	ClientState state = ClientState::Stopped;

	sockaddr_in serverAddress = {};
	SOCKET clientSocket = INVALID_SOCKET;

	std::string playerName;
	std::vector<Message> messages;
};

