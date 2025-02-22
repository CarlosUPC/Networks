#pragma once

#include "ModuleNetworking.h"

class ModuleNetworkingClient : public ModuleNetworking
{
public:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingClient public methods
	//////////////////////////////////////////////////////////////////////

	void setServerAddress(const char *serverAddress, uint16 serverPort);

	void setPlayerInfo(const char *playerName, uint8 spaceshipType);

	inline uint32 getPlayerNetworkID() const { return networkId; }

	inline void setInputDataFront(uint32 input)  {  inputDataFront = input; }

private:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworking virtual methods
	//////////////////////////////////////////////////////////////////////

	bool isClient() const override { return true; }

	void onStart() override;

	void onGui() override;

	void onPacketReceived(const InputMemoryStream &packet, const sockaddr_in &fromAddress) override;

	void onUpdate() override;

	void onConnectionReset(const sockaddr_in &fromAddress) override;

	void onDisconnect() override;



	//////////////////////////////////////////////////////////////////////
	// Client state
	//////////////////////////////////////////////////////////////////////

	enum class ClientState
	{
		Stopped,
		Connecting,
		Connected
	};

	ClientState state = ClientState::Stopped;

	std::string serverAddressStr;
	uint16 serverPort = 0;

	sockaddr_in serverAddress = {};
	std::string playerName = "player";
	uint8 spaceshipType = 0;

	uint32 playerId = 0;
	uint32 networkId = 0;

	ReplicationManagerClient replicationManager;
	DeliveryManager deliveryManager;
	// Connecting stage

	float secondsSinceLastHello = 0.0f;


	// Input ///////////

	static const int MAX_INPUT_DATA_SIMULTANEOUS_PACKETS = 64;

	InputPacketData inputData[MAX_INPUT_DATA_SIMULTANEOUS_PACKETS];
	uint32 inputDataFront = 0;
	uint32 inputDataBack = 0;

	float inputDeliveryIntervalSeconds = 0.05f;
	float secondsSinceLastInputDelivery = 0.0f;



	//////////////////////////////////////////////////////////////////////
	// Virtual connection
	//////////////////////////////////////////////////////////////////////

	// TODO(you): UDP virtual connection lab session

	double lastPacketReceivedTime = 0.0;
	float secondsSinceLastPing = 0.0f;
	float secondsDeathTimer = 0.0f;

	std::unordered_map<std::string, uint32> rank_list;
	bool died = false;
	//////////////////////////////////////////////////////////////////////
	// Replication
	//////////////////////////////////////////////////////////////////////

	// TODO(you): World state replication lab session



	//////////////////////////////////////////////////////////////////////
	// Delivery manager
	//////////////////////////////////////////////////////////////////////

	// TODO(you): Reliability on top of UDP lab session



	//////////////////////////////////////////////////////////////////////
	// Latency management
	//////////////////////////////////////////////////////////////////////

	// TODO(you): Latency management lab session

};

