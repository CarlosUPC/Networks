#include "Networks.h"


bool ModuleGameObject::init()
{
	return true;
}

bool ModuleGameObject::preUpdate()
{
	BEGIN_TIMED_BLOCK(GOPreUpdate);

	static const GameObject::State gNextState[] = {
		GameObject::NON_EXISTING, // After NON_EXISTING
		GameObject::STARTING,     // After INSTANTIATE
		GameObject::UPDATING,     // After STARTING
		GameObject::UPDATING,     // After UPDATING
		GameObject::DESTROYING,   // After DESTROY
		GameObject::NON_EXISTING  // After DESTROYING
	};

	for (GameObject &gameObject : gameObjects)
	{
		gameObject.state = gNextState[gameObject.state];

		if (App->modNetClient->isConnected())
		{

			if (gameObject.state == GameObject::UPDATING)
			{
				if (!gameObject.isLifebar)
				{
					gameObject.secondsElapsed += Time.deltaTime;

					float ratio = gameObject.secondsElapsed / REPLICATION_INTERVAL_SECONDS;

					if (ratio > 1)
						ratio = 1;

					gameObject.position = ((gameObject.final_position - gameObject.initial_position) * ratio) + gameObject.initial_position;
					gameObject.angle = ((gameObject.final_angle - gameObject.initial_angle) * ratio) + gameObject.initial_angle;

				}


				if (gameObject.lifebar != nullptr  /*gameObject.isLifebar*/)
				{
					gameObject.lifebar->position = gameObject.position + vec2{ -50.0f, -50.0f };

					const float alpha = 0.8f;
					const int height = 6;
					if (gameObject.life == 100)
					{
						gameObject.lifebar->sprite->color = vec4{ 0.0f, 1.0f, 0.0f, alpha };
						gameObject.lifebar->size = { 80, height };
					}
					else if (gameObject.life == 75)
					{
						gameObject.lifebar->sprite->color = vec4{ 1.0f, 1.0f, 0.0f, alpha };
						gameObject.lifebar->size = { 60, height };
					}
					else if (gameObject.life == 50)
					{
						gameObject.lifebar->sprite->color = vec4{ 1.0f, 0.75f, 0.0f, alpha };
						gameObject.lifebar->size = { 40, height };
					}
					else if (gameObject.life == 25)
					{
						gameObject.lifebar->sprite->color = vec4{ 1.0f, 0.5f, 0.0f, alpha };
						gameObject.lifebar->size = { 20, height };
					}
					else if (gameObject.life == 0)
					{
						gameObject.lifebar->sprite->color = vec4{ 1.0f, 0.0f, 0.0f, alpha };
						gameObject.lifebar->size = { 1, height };
					}
				}

			}
			
		}
	}

	END_TIMED_BLOCK(GOPreUpdate);

	return true;
}

bool ModuleGameObject::update()
{
	// Delayed destructions
	for (DelayedDestroyEntry &destroyEntry : gameObjectsWithDelayedDestruction)
	{
		if (destroyEntry.object != nullptr)
		{
			destroyEntry.delaySeconds -= Time.deltaTime;
			if (destroyEntry.delaySeconds <= 0.0f)
			{
				Destroy(destroyEntry.object->lifebar);
				destroyEntry.object->lifebar = nullptr;
				Destroy(destroyEntry.object);
				destroyEntry.object = nullptr;
			}
		}
	}

	return true;
}

bool ModuleGameObject::postUpdate()
{
	return true;
}

bool ModuleGameObject::cleanUp()
{
	return true;
}

GameObject * ModuleGameObject::Instantiate()
{
	for (uint32 i = 0; i < MAX_GAME_OBJECTS; ++i)
	{
		GameObject &gameObject = App->modGameObject->gameObjects[i];

		if (gameObject.state == GameObject::NON_EXISTING)
		{
			gameObject = GameObject();
			gameObject.id = i;
			gameObject.state = GameObject::INSTANTIATE;
			return &gameObject;
		}
	}

	ASSERT(0); // NOTE(jesus): You need to increase MAX_GAME_OBJECTS in case this assert crashes
	return nullptr;
}

void ModuleGameObject::Destroy(GameObject * gameObject)
{
	ASSERT(gameObject->networkId == 0); // NOTE(jesus): If it has a network identity, it must be destroyed by the Networking module first

	static const GameObject::State gNextState[] = {
		GameObject::NON_EXISTING, // After NON_EXISTING
		GameObject::DESTROY,      // After INSTANTIATE
		GameObject::DESTROY,      // After STARTING
		GameObject::DESTROY,      // After UPDATING
		GameObject::DESTROY,      // After DESTROY
		GameObject::DESTROYING    // After DESTROYING
	};

	ASSERT(gameObject->state < GameObject::STATE_COUNT);
	gameObject->state = gNextState[gameObject->state];
}

void ModuleGameObject::Destroy(GameObject * gameObject, float delaySeconds)
{
	for (uint32 i = 0; i < MAX_GAME_OBJECTS; ++i)
	{
		if (App->modGameObject->gameObjectsWithDelayedDestruction[i].object == nullptr)
		{
			App->modGameObject->gameObjectsWithDelayedDestruction[i].object = gameObject;
			App->modGameObject->gameObjectsWithDelayedDestruction[i].delaySeconds = delaySeconds;
			break;
		}
	}
}

GameObject * Instantiate()
{
	GameObject *result = ModuleGameObject::Instantiate();
	return result;
}

void Destroy(GameObject * gameObject)
{
	ModuleGameObject::Destroy(gameObject);
}

void Destroy(GameObject * gameObject, float delaySeconds)
{
	ModuleGameObject::Destroy(gameObject, delaySeconds);
}
