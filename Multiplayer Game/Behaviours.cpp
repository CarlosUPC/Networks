#include "Networks.h"
#include "Behaviours.h"



void Laser::start()
{
	gameObject->networkInterpolationEnabled = false;

	App->modSound->playAudioClip(App->modResources->audioClipLaser);
}

void Laser::update()
{
	secondsSinceCreation += Time.deltaTime;

	const float pixelsPerSecond = 1000.0f;
	gameObject->position += vec2FromDegrees(gameObject->angle) * pixelsPerSecond * Time.deltaTime;

	if (isServer)
	{
		NetworkUpdate(gameObject);

		const float neutralTimeSeconds = 0.1f;
		if (secondsSinceCreation > neutralTimeSeconds && gameObject->collider == nullptr) {
			gameObject->collider = App->modCollision->addCollider(ColliderType::Laser, gameObject);
		}

		const float lifetimeSeconds = 2.0f;
		if (secondsSinceCreation >= lifetimeSeconds) {
			NetworkDestroy(gameObject);
		}
	}
}





void Spaceship::start()
{
	gameObject->tag = (uint32)(Random.next() * UINT_MAX);

	lifebar = Instantiate();
	lifebar->sprite = App->modRender->addSprite(lifebar);
	lifebar->sprite->pivot = vec2{ 0.0f, 0.5f };
	lifebar->sprite->order = 1;
}

void Spaceship::onInput(const InputController &input)
{
	//ROTATE
	if (input.horizontalAxis != 0.0f)
	{
		const float rotateSpeed = 180.0f;
		gameObject->angle += input.horizontalAxis * rotateSpeed * Time.deltaTime;

		if (isServer)
		{
			NetworkUpdate(gameObject);
		}
	}
	//MOVEMENT
	if (input.actionDown == ButtonState::Pressed)
	{
		const float advanceSpeed = 200.0f;
		gameObject->position += vec2FromDegrees(gameObject->angle) * advanceSpeed * Time.deltaTime;

		if (isServer)
		{
			NetworkUpdate(gameObject);
		}
	}
	//BASIC ATTACK
	if (input.actionLeft == ButtonState::Press)
	{
		if (isServer)
		{
			GameObject* laser = NetworkInstantiate();

			laser->position = gameObject->position;
			laser->angle = gameObject->angle;
			laser->size = { 20, 60 };

			laser->sprite = App->modRender->addSprite(laser);
			laser->sprite->order = 1;
			laser->sprite->texture = App->modResources->laser;

			Laser* laserBehaviour = App->modBehaviour->addLaser(laser);
			laserBehaviour->isServer = isServer;

			laser->tag = gameObject->tag;
		}
	}

	//ULTIMATE
	const float cooldown = 5.0f;
	if (input.actionRight == ButtonState::Press && secondsSinceLastUltimate >= cooldown)
	{
		if (isServer)
		{
			spawnLaser(gameObject, gameObject->angle);
			spawnLaser(gameObject, gameObject->angle + 45.0f);
			spawnLaser(gameObject, gameObject->angle + 90.0f);
			spawnLaser(gameObject, gameObject->angle + 135.0f);
			spawnLaser(gameObject, gameObject->angle + 180.0f);
			spawnLaser(gameObject, gameObject->angle + 225.0f);
			spawnLaser(gameObject, gameObject->angle + 270.0f);
			spawnLaser(gameObject, gameObject->angle + 315.0f);
			spawnLaser(gameObject, gameObject->angle + 360.0f);

			

			secondsSinceLastUltimate = 0.0f;

			enable_ultimate = false;
			gameObject->ultimate = false;
			NetworkUpdate(gameObject);
		}
		
	}

	//Notify Client that has ultimate ready to use
	if (isServer)
	{
		if (secondsSinceLastUltimate >= cooldown && !enable_ultimate)
		{
			gameObject->ultimate = true;
			NetworkUpdate(gameObject);
			enable_ultimate = true;
		}
	}

}

void Spaceship::update()
{
	static const vec4 colorAlive = vec4{ 0.2f, 1.0f, 0.1f, 0.5f };
	static const vec4 colorDead = vec4{ 1.0f, 0.2f, 0.1f, 0.5f };
	const float lifeRatio = max(0.01f, (float)(hitPoints) / (MAX_HIT_POINTS));
	lifebar->position = gameObject->position + vec2{ -50.0f, -50.0f };
	lifebar->size = vec2{ lifeRatio * 80.0f, 5.0f };
	lifebar->sprite->color = lerp(colorDead, colorAlive, lifeRatio);

	secondsSinceLastUltimate += Time.deltaTime;
}

void Spaceship::destroy()
{
	Destroy(lifebar);
}

void Spaceship::onCollisionTriggered(Collider &c1, Collider &c2)
{
	if (c2.type == ColliderType::Laser && c2.gameObject->tag != gameObject->tag)
	{
		if (isServer)
		{
			NetworkDestroy(c2.gameObject); // Destroy the laser
		
			if (hitPoints > 0)
			{
				hitPoints--;
				NetworkUpdate(gameObject);
			}

			float size = 30 + 50.0f * Random.next();
			vec2 position = gameObject->position + 50.0f * vec2{Random.next() - 0.5f, Random.next() - 0.5f};

			if (hitPoints <= 0)
			{
				// Centered big explosion
				size = 250.0f + 100.0f * Random.next();
				position = gameObject->position;

				// Get kill point
				GameObject* killer = App->modNetServer->getNetworkObjectByTag(c2.gameObject->tag);
				if (killer != nullptr)
				{
					++killer->kills;
					NetworkUpdate(killer);
				}


				// Victim died
				gameObject->die = true;
				//NetworkUpdate(gameObject);
				//NetworkDestroy(gameObject); //Commented for disconnect client (maybe is not good idea)
			}

			GameObject *explosion = NetworkInstantiate();
			explosion->position = position;
			explosion->size = vec2{ size, size };
			explosion->angle = 365.0f * Random.next();

			explosion->sprite = App->modRender->addSprite(explosion);
			explosion->sprite->texture = App->modResources->explosion1;
			explosion->sprite->order = 100;

			explosion->animation = App->modRender->addAnimation(explosion);
			explosion->animation->clip = App->modResources->explosionClip;

			NetworkDestroy(explosion, 2.0f);

			// NOTE(jesus): Only played in the server right now...
			// You need to somehow make this happen in clients
			App->modSound->playAudioClip(App->modResources->audioClipExplosion);
		}
	}
}

void Spaceship::write(OutputMemoryStream & packet)
{
	packet << hitPoints;
}

void Spaceship::read(const InputMemoryStream & packet)
{
	packet >> hitPoints;
}

void Spaceship::spawnLaser(GameObject* object, float angle)
{
	GameObject* laser = NetworkInstantiate();

	laser->position = gameObject->position;
	laser->angle = angle;
	laser->size = { 20, 60 };

	laser->sprite = App->modRender->addSprite(laser);
	laser->sprite->order = 1;
	laser->sprite->texture = App->modResources->laser;

	Laser* laserBehaviour = App->modBehaviour->addLaser(laser);
	laserBehaviour->isServer = isServer;

	laser->tag = gameObject->tag;
}
