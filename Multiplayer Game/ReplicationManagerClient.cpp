#include "Networks.h"

// TODO(you): World state replication lab session

void ReplicationManagerClient::read(const InputMemoryStream& packet, ModuleNetworkingClient* client)
{

	while (packet.RemainingByteCount() > 0)
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
				packet >> gameObject->life;

				gameObject->final_position = gameObject->initial_position = gameObject->position;
				gameObject->final_angle = gameObject->initial_angle = gameObject->angle;

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
					gameObject->sprite->order = 1;
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
					gameObject->sprite->order = 2;

					//Add lifebar
					if (gameObject->lifebar == nullptr)
					{
						gameObject->lifebar = Instantiate();
						//App->modLinkingContext->registerNetworkGameObjectWithNetworkId(gameObject->lifebar, networkId);
						gameObject->lifebar->sprite = App->modRender->addSprite(gameObject->lifebar);
						gameObject->lifebar->sprite->pivot = vec2{ 0.0f, 0.5f };
						gameObject->lifebar->sprite->order = 1;
						gameObject->lifebar->isLifebar = true;
					}
				}


			}


		}
		else if (action == ReplicationAction::Update)
		{
			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkId);
			if (gameObject != nullptr)
			{
				

				/*if (networkId == App->modNetClient->getPlayerNetworkID())
				{
					packet >> gameObject->position.x;
					packet >> gameObject->position.y;
					packet >> gameObject->angle;
					packet >> gameObject->kills;
					packet >> gameObject->die;
					packet >> gameObject->ultimate;

					
					

					continue;
				}*/

				packet >> gameObject->final_position.x;
				packet >> gameObject->final_position.y;
				packet >> gameObject->final_angle;
				packet >> gameObject->kills;
				packet >> gameObject->ultimate;
				packet >> gameObject->life;

				
				

				gameObject->initial_position = gameObject->position;
				gameObject->initial_angle = gameObject->angle;


				

				gameObject->secondsElapsed = 0.0f;

				
			}

		}
		else if (action == ReplicationAction::Input)
		{
			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkId);
			if (gameObject != nullptr)
			{
				//TODO: Set input data to clients
				
					
					uint32 data = 0u;
					packet >> data;

					client->setInputDataFront(data);

				

			}
		}

		else if (action == ReplicationAction::Destroy)
		{
			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkId);
			if (gameObject != nullptr)
			{

				if (gameObject->lifebar != nullptr)
				{
					//App->modLinkingContext->unregisterNetworkGameObject(gameObject->lifebar);
					Destroy(gameObject->lifebar);
					gameObject->lifebar = nullptr;
				}

				App->modLinkingContext->unregisterNetworkGameObject(gameObject);
				Destroy(gameObject);
			}
		}

	}
}