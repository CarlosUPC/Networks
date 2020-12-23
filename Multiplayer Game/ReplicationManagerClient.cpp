#include "Networks.h"

// TODO(you): World state replication lab session

void ReplicationManagerClient::read(const InputMemoryStream& packet)
{

	while (packet.RemainingByteCount() > sizeof(uint32))
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
				//Register it
				App->modLinkingContext->registerNetworkGameObjectWithNetworkId(gameObject, networkId);


				//Add Basics
				packet >> gameObject->position.x;
				packet >> gameObject->position.y;
				packet >> gameObject->angle;
				packet >> gameObject->size.x;
				packet >> gameObject->size.y;


				//Add Sprite
				gameObject->sprite = App->modRender->addSprite(gameObject);

				uint8 spaceShipType;
				packet >> spaceShipType;

				switch (spaceShipType)
				{
				case 0:
					gameObject->sprite->texture = App->modResources->laser;
					break;
				case 1:
					gameObject->sprite->texture = App->modResources->spacecraft1;
					break;
				case 2:
					gameObject->sprite->texture = App->modResources->spacecraft2;
					break;
				case 3:
					gameObject->sprite->texture = App->modResources->spacecraft3;
					break;
				}


				if (spaceShipType == 0)//Laser
				{
					//Add Collider
					gameObject->collider = App->modCollision->addCollider(ColliderType::Laser, gameObject);
					gameObject->collider->isTrigger = true;

					//Add Behaviour
					//gameObject->behaviour = new Laser();
					//gameObject->behaviour->gameObject = gameObject;

				}
				else //Player
				{
					//Add Collider
					gameObject->collider = App->modCollision->addCollider(ColliderType::Player, gameObject);
					gameObject->collider->isTrigger = true;

					//Add Behaviour
					gameObject->behaviour = new Spaceship();
					gameObject->behaviour->gameObject = gameObject;
				}


			}


		}
		else if (action == ReplicationAction::Update)
		{
			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkId);
			if (gameObject != nullptr)
			{
				//packet >> gameObject->position.x;
				//packet >> gameObject->position.y;
				//packet >> gameObject->angle;

				//vec2 _position;
				//float _angle;

				if (networkId == App->modNetClient->getPlayerNetworkID())
				{
					packet >> gameObject->position.x;
					packet >> gameObject->position.y;
					packet >> gameObject->angle;

					continue;
				}

				packet >> gameObject->final_position.x;
				packet >> gameObject->final_position.y;
				packet >> gameObject->final_angle;

				gameObject->initial_position = gameObject->position;
				gameObject->initial_angle = gameObject->angle;


				//gameObject->final_position = _position;
				//gameObject->final_angle = _angle;

				gameObject->secondsElapsed = .0f;
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
}