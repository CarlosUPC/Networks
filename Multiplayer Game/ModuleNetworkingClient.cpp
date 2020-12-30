#include "ModuleNetworkingClient.h"


//////////////////////////////////////////////////////////////////////
// ModuleNetworkingClient public methods
//////////////////////////////////////////////////////////////////////


void ModuleNetworkingClient::setServerAddress(const char * pServerAddress, uint16 pServerPort)
{
	serverAddressStr = pServerAddress;
	serverPort = pServerPort;
}

void ModuleNetworkingClient::setPlayerInfo(const char * pPlayerName, uint8 pSpaceshipType)
{
	playerName = pPlayerName;
	spaceshipType = pSpaceshipType;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

void ModuleNetworkingClient::onStart()
{
	if (!createSocket()) return;

	if (!bindSocketToPort(0)) {
		disconnect();
		return;
	}

	// Create remote address
	serverAddress = {};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(serverPort);
	int res = inet_pton(AF_INET, serverAddressStr.c_str(), &serverAddress.sin_addr);
	if (res == SOCKET_ERROR) {
		reportError("ModuleNetworkingClient::startClient() - inet_pton");
		disconnect();
		return;
	}

	//RESET STUFF FOR RECONNECTION 
	state = ClientState::Connecting;

	inputDataFront = 0;
	inputDataBack = 0;

	secondsSinceLastHello = 9999.0f;
	secondsSinceLastPing = 0.0f;
	secondsSinceLastInputDelivery = 0.0f;
	secondsDeathTimer = 0.0f;
	
	died = false;
	lastPacketReceivedTime = Time.time;

	replicationManager = {};
	deliveryManager = {};
}

void ModuleNetworkingClient::onGui()
{
	if (state == ClientState::Stopped) return;

	if (ImGui::CollapsingHeader("ModuleNetworkingClient", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (state == ClientState::Connecting)
		{
			ImGui::Text("Connecting to server...");
		}
		else if (state == ClientState::Connected)
		{
			ImGui::Text("Connected to server");

			ImGui::Separator();

			ImGui::Text("Player info:");
			ImGui::Text(" - Id: %u", playerId);
			ImGui::Text(" - Name: %s", playerName.c_str());

			ImGui::Separator();

			ImGui::Text("Spaceship info:");
			ImGui::Text(" - Type: %u", spaceshipType);
			ImGui::Text(" - Network id: %u", networkId);

			vec2 playerPosition = {};
			GameObject *playerGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
			if (playerGameObject != nullptr) {
				playerPosition = playerGameObject->position;
			}
			ImGui::Text(" - Coordinates: (%f, %f)", playerPosition.x, playerPosition.y);

			ImGui::Separator();

			ImGui::Text("Connection checking info:");
			ImGui::Text(" - Ping interval (s): %f", PING_INTERVAL_SECONDS);
			ImGui::Text(" - Disconnection timeout (s): %f", DISCONNECT_TIMEOUT_SECONDS);

			ImGui::Separator();

			ImGui::Text("Input:");
			ImGui::InputFloat("Delivery interval (s)", &inputDeliveryIntervalSeconds, 0.01f, 0.1f, 4);
		}
	}

	if (state == ClientState::Connected) {
		ImGui::SetNextWindowPos(ImVec2(320.0f, 15.0f));
		ImGui::SetNextWindowSize(ImVec2(200.0f, 115.0f));

		GameObject* player = App->modLinkingContext->getNetworkGameObject(networkId);
		if (ImGui::Begin("Player"))
		{

			if (ImGui::CollapsingHeader("Score", ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (player != nullptr) {
					ImGui::Text(" - Kills: %u", player->kills);
				}
			}
			if (ImGui::CollapsingHeader("PowerUp", ImGuiTreeNodeFlags_DefaultOpen))
			{
				//GameObject* player = App->modLinkingContext->getNetworkGameObject(networkId);
				if (player != nullptr) {
					if(!player->ultimate)
						ImGui::Text(" - Ultimate in cooldown");
					else
					{
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
						ImGui::Text(" - ULTIMATE READY");
						ImGui::PopStyleColor();
					}
					
				}

			}
		}
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(820.0f, 15.0f));
		ImGui::SetNextWindowSize(ImVec2(150.0f, 115.0f));
		if (ImGui::Begin("Ranking"))
		{
			int i = 1;
			for (std::pair<std::string, uint32> player : rank_list)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
				ImGui::Text("[%u] - %s ", i, player.first.c_str());
				ImGui::PopStyleColor();
				ImGui::Text(" - Kills: %u ", player.second);
				
				++i;
			}
			
		}
		ImGui::End();

		if (died)
		{
			ImGui::OpenPopup("GAME OVER");

			if (ImGui::BeginPopupModal("GAME OVER", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
			{


				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
				ImVec2 cursorPos = ImGui::GetCursorScreenPos();
				ImGui::SetCursorPosX(180 / 2 - ImGui::CalcTextSize("GAME OVER").x / 2);
				ImGui::Text("GAME OVER");
				ImGui::Separator();
				ImGui::PopStyleColor();

				ImGui::NewLine();

				if (ImGui::Button("Return to lobby", { 170, 35 }))
				{
					secondsDeathTimer = 0.0f;
					disconnect();
				}

				const float death_time = 5.0f;
				if (secondsDeathTimer >= death_time)
				{
					secondsDeathTimer = 0.0f;
					disconnect();
				}
				secondsDeathTimer += Time.deltaTime;


				ImGui::EndPopup();
			}
		}
		
	}
}

void ModuleNetworkingClient::onPacketReceived(const InputMemoryStream &packet, const sockaddr_in &fromAddress)
{
	// TODO(you): UDP virtual connection lab session
	lastPacketReceivedTime = Time.time;
	
	uint32 protoId;
	packet >> protoId;
	if (protoId != PROTOCOL_ID) return;

	ServerMessage message;
	packet >> message;

	if (state == ClientState::Connecting)
	{
		if (message == ServerMessage::Welcome)
		{
			packet >> playerId;
			packet >> networkId;

			LOG("ModuleNetworkingClient::onPacketReceived() - Welcome from server");
			state = ClientState::Connected;
		}
		else if (message == ServerMessage::Unwelcome)
		{
			WLOG("ModuleNetworkingClient::onPacketReceived() - Unwelcome from server :-(");
			disconnect();
		}
	}
	else if (state == ClientState::Connected)
	{
		if (message == ServerMessage::Ping)
		{
			//lastPacketReceivedTime = Time.time;
		}
		
		// TODO(you): World state replication lab session
		if (message == ServerMessage::Replication)
		{
			if (deliveryManager.processSequenceNumber(packet))
				replicationManager.read(packet, this);

			

			//CLIENT SIDE PREDICTION ---- Reapply inputs not processed by the server

			GameObject* playerGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
			if (playerGameObject != nullptr)
			{
				for (int i = inputDataFront; i != inputDataBack; ++i)
				{
					InputPacketData inputPacketData = inputData[i];
					InputController inputController = inputControllerFromInputPacketData(inputPacketData);

					playerGameObject->behaviour->onInput(Input);
				}
			}
		}

		if (message == ServerMessage::Ranking)
		{
			rank_list.clear();

			while (packet.RemainingByteCount() > 0)
			{
				std::string name;
				uint32 kills;

				packet >> name;
				packet >> kills;

				if (rank_list.find(name) == rank_list.end())
					rank_list[name] = kills;
			}
		}

		if (message == ServerMessage::Death)
		{
			died = true;
		}
		// TODO(you): Reliability on top of UDP lab session
	}
}

void ModuleNetworkingClient::onUpdate()
{
	if (state == ClientState::Stopped) return;


	// TODO(you): UDP virtual connection lab session

	
	if (state == ClientState::Connecting)
	{
		secondsSinceLastHello += Time.deltaTime;

		if (secondsSinceLastHello > 0.1f)
		{
			secondsSinceLastHello = 0.0f;

			OutputMemoryStream packet;
			packet << PROTOCOL_ID;
			packet << ClientMessage::Hello;
			packet << playerName;
			packet << spaceshipType;

			sendPacket(packet, serverAddress);

		}
	}
	else if (state == ClientState::Connected)
	{
		// TODO(you): UDP virtual connection lab session

		secondsSinceLastPing += Time.deltaTime;
		secondsSinceLastInputDelivery += Time.deltaTime;

		if (inputDataBack - inputDataFront <= ArrayCount(inputData))
		{
			//Create new input packet
			uint32 currentInputData = inputDataBack++;
			InputPacketData& inputPacketData = inputData[currentInputData % ArrayCount(inputData)];
			inputPacketData.sequenceNumber = currentInputData;
			inputPacketData.horizontalAxis = Input.horizontalAxis;
			inputPacketData.verticalAxis = Input.verticalAxis;
			inputPacketData.buttonBits = packInputControllerButtons(Input);

			//Process the new input: Client Side
			/*GameObject* playerGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
			if (playerGameObject != nullptr)
			{
				playerGameObject->behaviour->onInput(Input);
			}*/

			// Create packet (if there's input and the input delivery interval exceeded)
			if (secondsSinceLastInputDelivery >= inputDeliveryIntervalSeconds)
			{
				secondsSinceLastInputDelivery = 0.0f;

				OutputMemoryStream packet;
				packet << PROTOCOL_ID;
				packet << ClientMessage::Input;

				for (uint32 i = inputDataFront; i < inputDataBack; ++i)
				{
					InputPacketData& inputPacketData = inputData[i % ArrayCount(inputData)];
					packet << inputPacketData.sequenceNumber;
					packet << inputPacketData.horizontalAxis;
					packet << inputPacketData.verticalAxis;
					packet << inputPacketData.buttonBits;
				}

				// Clear the queue
				//inputDataFront = inputDataBack;

				sendPacket(packet, serverAddress);
			}
		}
		else inputDataFront = inputDataBack;

		if (Time.time - lastPacketReceivedTime >= DISCONNECT_TIMEOUT_SECONDS)
		{
			disconnect();
			DLOG("Time of last packed timedout");
		}

		if (secondsSinceLastPing >= PING_INTERVAL_SECONDS)
		{
			secondsSinceLastPing = 0.0f;

			OutputMemoryStream packet;
			packet << PROTOCOL_ID;
			packet << ClientMessage::Ping;

			deliveryManager.writeSequenceNumbersPendingAck(packet); //Send ack numbers to server
			deliveryManager.processTimedOutPackets();

			sendPacket(packet, serverAddress);
			DLOG("Client send Ping message");

		}

		
		// TODO(you): Latency management lab session

		// Update camera for player
		GameObject *playerGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
		if (playerGameObject != nullptr)
		{
			App->modRender->cameraPosition = playerGameObject->position;
		
		}
		else
		{
			// This means that the player has been destroyed (e.g. killed)
		}
	}
}

void ModuleNetworkingClient::onConnectionReset(const sockaddr_in & fromAddress)
{
	disconnect();
}

void ModuleNetworkingClient::onDisconnect()
{
	state = ClientState::Stopped;

	GameObject *networkGameObjects[MAX_NETWORK_OBJECTS] = {};
	uint16 networkGameObjectsCount;
	App->modLinkingContext->getNetworkGameObjects(networkGameObjects, &networkGameObjectsCount);
	App->modLinkingContext->clear();

	for (uint32 i = 0; i < networkGameObjectsCount; ++i)
	{
		Destroy(networkGameObjects[i]);
	}

	App->modRender->cameraPosition = {};
}
