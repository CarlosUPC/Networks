#include "Networks.h"

// TODO(you): World state replication lab session

void ReplicationManagerClient::read(const InputMemoryStream& packet)
{
	uint32 networkId;
	ReplicationAction action;
	
	packet >> networkId;
	packet >> action;

	if (action == ReplicationAction::Create)
	{
		GameObject* gameObject = Instantiate();
		if (gameObject != nullptr)
		{
			App->modLinkingContext->registerNetworkGameObjectWithNetworkId(gameObject, networkId);

			packet >> gameObject->position.x;
			packet >> gameObject->position.y;
			packet >> gameObject->angle;
		}
	}
	else if (action == ReplicationAction::Update)
	{	
		GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkId);
		if (gameObject != nullptr)
		{
			packet >> gameObject->position.x;
			packet >> gameObject->position.y;
			packet >> gameObject->angle;
		}

	}
	else if (action == ReplicationAction::Destroy)
	{
		GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkId);
		if (gameObject != nullptr)
		{
			App->modLinkingContext->unregisterNetworkGameObject(gameObject);
			Destroy(gameObject);
		}
	}
}