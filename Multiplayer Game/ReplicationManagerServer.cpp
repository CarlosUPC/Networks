#include "Networks.h"
#include "ReplicationManagerServer.h"

// TODO(you): World state replication lab session

void ReplicationManagerServer::create(uint32 networkId)
{
	ReplicationCommand command;
	command.action = ReplicationAction::Create;
	command.networkId = networkId;

	//map.insert(std::pair<uint32, ReplicationCommand>(networkId, command));

	if (map.find(networkId) == map.end())
		map[networkId] = command;
}

void ReplicationManagerServer::update(uint32 networkId)
{
	ReplicationCommand command;
	command.action = ReplicationAction::Update;
	command.networkId = networkId;

	//map.insert(std::pair<uint32, ReplicationCommand>(networkId, command));

	if (map.find(networkId) == map.end())
		map[networkId] = command;
}

void ReplicationManagerServer::destroy(uint32 networkId)
{
	ReplicationCommand command;
	command.action = ReplicationAction::Destroy;
	command.networkId = networkId;

	//map.insert(std::pair<uint32, ReplicationCommand>(networkId, command));

	//if (map.find(networkId) == map.end())
	map[networkId] = command;
}


void ReplicationManagerServer::write(OutputMemoryStream& packet)
{
	
	for (std::unordered_map<uint32, ReplicationCommand>::iterator it = map.begin(); it != map.end(); ++it)
	{
		ReplicationCommand& command = it->second;
		packet << command.networkId;
		packet << command.action;
		


		if (command.action == ReplicationAction::Create)
		{
			//Get the object from LinkingContext
			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(command.networkId);

			//Serialize fields
			if (gameObject != nullptr)
			{
				packet << gameObject->position.x;
				packet << gameObject->position.y;
				packet << gameObject->angle;
				packet << gameObject->size.x;
				packet << gameObject->size.y;

				if (gameObject->sprite->texture == App->modResources->spacecraft1)
					packet << (uint8)1;
				else if (gameObject->sprite->texture == App->modResources->spacecraft2)
					packet << (uint8)2;
				else if (gameObject->sprite->texture == App->modResources->spacecraft3)
					packet << (uint8)3;
				else
					packet << (uint8)0;
			}
		}
		else if (command.action == ReplicationAction::Update)
		{
			//Get the object from LinkingContext
			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(command.networkId);

			//Serialize fields
			if (gameObject != nullptr)
			{
				packet << gameObject->position.x;
				packet << gameObject->position.y;
				packet << gameObject->angle;
				packet << gameObject->kills;
				packet << gameObject->die;
			}

		}
		else if (command.action == ReplicationAction::Destroy)
		{
			//Nothing else to do
		}
	}

	map.clear(); //With this we are assuming reliability.
}

bool ReplicationManagerServer::isEmpty() const
{
	return map.empty();
}
