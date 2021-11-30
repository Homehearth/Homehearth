#include "NetServerPCH.h"
#include "Wave.h"
#include "ServerSystems.h"
#include "Simulation.h"


/**Creates an enemy entity at specified point.
 *
 *@param simulation Creates entity on current scene, and adds the network component
 *@param spawnP     Specifies the point at which the entity is to be created at
 *@param type       Specifies the type of enemy to create
 *@return Entity	returns the enemy that has been created as an Entity
 */
Entity EnemyManagement::CreateEnemy(Simulation* simulation, sm::Vector3 spawnP, EnemyType type)
{
	Entity entity = simulation->GetGameScene()->CreateEntity();
	entity.AddComponent<comp::Network>();
	comp::NPC* npc = entity.AddComponent<comp::NPC>();
	entity.AddComponent<comp::Tag<DYNAMIC>>();
	entity.AddComponent<comp::Tag<BAD>>(); // this entity is BAD

	comp::Transform* transform = entity.AddComponent<comp::Transform>();
	comp::Health* health = entity.AddComponent<comp::Health>();
	comp::MeshName* meshName = entity.AddComponent<comp::MeshName>();
	comp::AnimatorName* animatorName = entity.AddComponent<comp::AnimatorName>();
	comp::AnimationState* animationState = entity.AddComponent<comp::AnimationState>();
	comp::SphereCollider* bos = entity.AddComponent<comp::SphereCollider>();
	comp::Velocity* velocity = entity.AddComponent<comp::Velocity>();
	comp::BehaviorTree* behaviorTree = entity.AddComponent<comp::BehaviorTree>();
	switch (type)
	{
	case EnemyType::Default:
	{
		comp::MeleeAttackAbility* attackAbility = entity.AddComponent<comp::MeleeAttackAbility>();
		// ---DEFAULT ENEMY---
		transform->position = spawnP;
		//Generate float between 0.0 and 0.5 (give monster a slightly different height?)
		float randomNum = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (0.5)));

		transform->scale = { 1.8f, 1.8f + randomNum, 1.8f };
		meshName->name = NameType::MESH_MONSTER;
		animatorName->name = AnimName::ANIM_MONSTER;

		bos->Radius = 3.f;

		npc->movementSpeed = 15.f;
		attackAbility->cooldown = 1.0f;
		attackAbility->attackDamage = 20.f;
		attackAbility->lifetime = 5.0f;
		attackAbility->attackRange = 10.f;
		attackAbility->useTime = 0.3f;
		attackAbility->delay = 0.2f;
		attackAbility->movementSpeedAlt = 0.0f;

		if (randomNum > 0.25f)
		{
			behaviorTree->root = AIBehaviors::GetFocusBuildingAIBehavior(entity);
		}
		else
		{
			behaviorTree->root = AIBehaviors::GetFocusPlayerAIBehavior(entity);
		}

	}
	break;
	case EnemyType::Mage:
	{
		comp::RangeAttackAbility* attackAbility = entity.AddComponent<comp::RangeAttackAbility>();
		comp::BlinkAbility* teleportAbility = entity.AddComponent<comp::BlinkAbility>();
		// ---DEFAULT ENEMY---
		transform->position = spawnP;
		//Generate float between 0.0 and 0.5 (give monster a slightly different height?)
		float randomNum = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (0.5)));

		transform->scale = { 1.8f, 0.5f + randomNum, 1.8f };
		meshName->name = NameType::MESH_MONSTER;
		animatorName->name = AnimName::ANIM_MONSTER;
		bos->Radius = 3.f;

		npc->movementSpeed = 15.f;
		attackAbility->cooldown = 3.0f;
		attackAbility->attackDamage = 20.f;
		attackAbility->lifetime = 5.0f;
		attackAbility->attackRange = 60.f;
		attackAbility->useTime = 0.3f;
		attackAbility->delay = 0.2f;
		attackAbility->projectileSpeed = 50.f;
		attackAbility->movementSpeedAlt = 0.0f;
		behaviorTree->root = AIBehaviors::GetFocusPlayerAIBehavior(entity);
	}
	break;
	case EnemyType::Runner:
	{
		comp::MeleeAttackAbility* attackAbility = entity.AddComponent<comp::MeleeAttackAbility>();
		// ---Fast Zombie ENEMY---
		transform->position = spawnP;
		transform->scale = { 1.8f, 3.f, 1.8f };
		meshName->name = NameType::MESH_MONSTER;
		animatorName->name = AnimName::ANIM_MONSTER;
		bos->Radius = 3.f;
		attackAbility->cooldown = 1.0f;
		attackAbility->attackDamage = 20.f;
		attackAbility->lifetime = 0.3f;
		attackAbility->attackRange = 7.0f;
		attackAbility->useTime = 0.3f;
		attackAbility->delay = 0.2f;
		attackAbility->movementSpeedAlt = 0.0f;
		npc->movementSpeed = 30.f;
		behaviorTree->root = AIBehaviors::GetFocusPlayerAIBehavior(entity);
	}
	break;
	case EnemyType::BIGMOMMA:
	{
		comp::MeleeAttackAbility* attackAbility = entity.AddComponent<comp::MeleeAttackAbility>();
		// ---BOSS ENEMY---
		transform->position = spawnP;
		transform->scale = { 3.8f, 6.f, 3.8f };
		meshName->name = NameType::MESH_MONSTER;
		animatorName->name = AnimName::ANIM_MONSTER;
		bos->Radius = 3.f;
		attackAbility->cooldown = 1.0f;
		attackAbility->attackDamage = 20.f;
		attackAbility->lifetime = 0.3f;
		attackAbility->attackRange = 20.0f;
		attackAbility->useTime = 0.3f;
		attackAbility->delay = 0.2f;
		attackAbility->movementSpeedAlt = 0.0f;
		npc->movementSpeed = 10.f;
		health->currentHealth = 1500.f;
		behaviorTree->root = AIBehaviors::GetFocusPlayerAIBehavior(entity);
	}
	break;
	default:
		LOG_WARNING("Attempted to create unknown EnemyType.")
			break;
	}

	return entity;
}

void EnemyManagement::CreateWaves(std::queue<Wave>& waveQueue, int currentRound)
{
	//Reeset wavequeu
	while (!waveQueue.empty())
	{
		waveQueue.pop();
	}

	//left bottom corner { 490.f, -150.0f });   right bottom corner { 170, -80.0f }
	//left top corner { 520.f, -540.0f }        right top corner { 80.0f, -500.0f }
	Wave wave1, wave2, wave3, wave4, wave5; // Default: WaveType::Zone
	{
		Wave::Group group1;
		group1.AddEnemy(EnemyType::Default, 2 + 2 * currentRound);
		group1.SetSpawnPoint({ 490.f, -150.0f });
		wave1.SetTimeLimit(5 * currentRound);
		wave1.AddGroup(group1);
	}

	{ // Wave_2
		Wave::Group group1, group2;

		group1.AddEnemy(EnemyType::Default, 1 + currentRound);
		group2.AddEnemy(EnemyType::Default, 2 + currentRound);
		group2.AddEnemy(EnemyType::Runner, 1 + 2 * currentRound);
		group1.SetSpawnPoint({ 490.f, -150.0f });
		group2.SetSpawnPoint({ 170, -80.0f });
		wave2.AddGroup(group1);
		wave2.AddGroup(group2);
		wave2.SetTimeLimit(30);	
	}


	{ // Wave_3
		Wave::Group group1, group2, group3, group4;

		group1.AddEnemy(EnemyType::Default, 3 + currentRound);
		group1.AddEnemy(EnemyType::Runner, 1 + 1 * currentRound);
		group1.SetSpawnPoint({ 490.f, -150.0f });

		group2.AddEnemy(EnemyType::Default, 3 + currentRound);
		group2.AddEnemy(EnemyType::Runner, 1 + currentRound);
		group2.SetSpawnPoint({ 170, -80.0f });

		group3.AddEnemy(EnemyType::Default, 3 + currentRound);
		group3.SetSpawnPoint({ 80.0f, -500.0f });

		group4.AddEnemy(EnemyType::Default, 2 + currentRound);
		group4.SetSpawnPoint({ 520.f, -540.0f });

		wave3.AddGroup(group1);
		wave3.AddGroup(group2);
		wave3.AddGroup(group3);
		wave3.AddGroup(group4);
		wave3.SetTimeLimit(45);
	}

	{ // Wave_4
		Wave::Group group1, group2, group3, group4;

		group1.AddEnemy(EnemyType::Default, 4 + currentRound);
		group1.AddEnemy(EnemyType::Runner, 1 + currentRound);
		group1.SetSpawnPoint({ 490.f, -150.0f });

		group2.AddEnemy(EnemyType::Default, 4 + currentRound);
		group2.AddEnemy(EnemyType::Runner, 1 + currentRound);
		group2.SetSpawnPoint({ 170, -80.0f });

		group3.AddEnemy(EnemyType::Default, 2 + currentRound);
		group3.AddEnemy(EnemyType::Runner, 2 + currentRound);
		group3.SetSpawnPoint({ 80.0f, -500.0f });

		group4.AddEnemy(EnemyType::Default, 4 + currentRound);
		group4.AddEnemy(EnemyType::Runner, 1 + currentRound);
		group4.SetSpawnPoint({ 520.f, -540.0f });

		wave4.AddGroup(group1);
		wave4.AddGroup(group2);
		wave4.AddGroup(group3);
		wave4.AddGroup(group4);
		wave4.SetTimeLimit(45);
	}

	{ // Wave_5 BOSS
		Wave::Group group1, group2, group3, group4;

		group1.AddEnemy(EnemyType::Mage, 2 + currentRound);
		group1.AddEnemy(EnemyType::BIGMOMMA, 1);
		group1.SetSpawnPoint({ 490.f, -150.0f });

		group2.AddEnemy(EnemyType::Default, 1 + currentRound);
		group2.SetSpawnPoint({ 170, -80.0f });

		group3.AddEnemy(EnemyType::Default, 2 + currentRound);
		group3.SetSpawnPoint({ 80.0f, -500.0f });

		group4.AddEnemy(EnemyType::Default, 1 + currentRound);
		group4.SetSpawnPoint({ 520.f, -540.0f });

		wave5.AddGroup(group1);
		wave5.AddGroup(group2);
		wave5.AddGroup(group3);
		wave5.AddGroup(group4);
		wave5.SetTimeLimit(45);
	}

	waveQueue.emplace(wave1);
	waveQueue.emplace(wave2);
	waveQueue.emplace(wave3);
	waveQueue.emplace(wave4);
	waveQueue.emplace(wave5);
}


/**Spawn the enemies randomly within a circular zone based on the specified point
 *
 *@param simulation	   Manages sending and removal of entities on the server.
 *@param point		   This is the point the calculation assumes to spawn the enemy from.
 *@param waveInfo      Contains configurations for the wave system.
 */
void SpawnZoneWave(Simulation* simulation, Wave& currentWave)
{
	LOG_INFO("[WaveSystem] spawns a zone wave...");

	const int min = 30;
	const int max = 70;
	const float deg2rad = 3.14f / 180.f;
	int nrOfEnemies = 0;
	float degree = 0.0f;

	float distance;
	float posX;
	float posZ;

	using namespace EnemyManagement;
	for (auto& group : currentWave.GetGroups())
	{
		//Go through each enemyType of that group
		constexpr int SIZE = static_cast<int>(EnemyType::ENUM_SIZE);
		for (auto it = 0; it < SIZE; it++)
		{
			nrOfEnemies = group.GetEnemyTypeCount(static_cast<EnemyType>(it));
			const float degree = (360.f / static_cast<float>(nrOfEnemies)) * deg2rad;

			//Spawn enemies of this type
			for (int i = 0; i < nrOfEnemies; i++)
			{
				distance = static_cast<float>(rand() % (max - min) + min);
				posX = distance * cos(i * degree) + group.GetSpawnPoint().x;
				posZ = distance * sin(i * degree) + group.GetSpawnPoint().y;
				EnemyManagement::CreateEnemy(simulation, { posX, 0.0f, posZ }, static_cast<EnemyType>(it));
			}
		}
	}
}

/**Spawn the enemies within a circular zone based on the specified point
 *
 *@param simulation	   Manages sending and removal of entities on the server.
 *@param point		   This is the point the calculation assumes to spawn the enemy from.
 *@param waveInfo      Contains configurations for the wave system.
 */
void SpawnSwarmWave(Simulation* simulation, Wave& currentWave)
{
	LOG_INFO("[WaveSystem] spawns a swarm wave...");

	const float distance = currentWave.GetDistance();
	const float deg2rad = 3.14f / 180.f;
	int nrOfEnemies = 0;

	float posX;
	float posZ;

	using namespace EnemyManagement;
	for (auto& group : currentWave.GetGroups())
	{
		//Go through each enemyType of that group
		constexpr int SIZE = static_cast<int>(EnemyType::ENUM_SIZE);
		for (auto it = 0; it < SIZE; it++)
		{
			nrOfEnemies = group.GetEnemyTypeCount(static_cast<EnemyType>(it));
			const float degree = (360.f / static_cast<float>(nrOfEnemies)) * deg2rad;

			for (int i = 0; i < nrOfEnemies; i++)
			{
				posX = distance * cos(i * degree) + group.GetSpawnPoint().x;
				posZ = distance * sin(i * degree) + group.GetSpawnPoint().y;
				EnemyManagement::CreateEnemy(simulation, { posX, 0.0f, posZ }, static_cast<EnemyType>(it));
			}
		}
	}
}

/**Wave system handles enemies spawns in the scene as a "wave", handles several different types of waves that is specified by the input queue.
 *
 *@param simulation	   Manages sending and removal of entities on the server.
 *@param waves         Queue containing std::pair that describes which wave type to spawn and which position in the world the wave should be based on.
 *@param waveInfo      Contains configurations for the wave system.
 */
void ServerSystems::WaveSystem(Simulation* simulation,
	std::queue<Wave>& waves)
{
	//initialize a new wave
	if (!waves.empty())
	{
		const EnemyManagement::WaveType waveType = waves.front().GetWaveType();

		switch (waveType)
		{
		case EnemyManagement::WaveType::Zone:
		{
			SpawnZoneWave(simulation, waves.front());
		}
		break;

		case EnemyManagement::WaveType::Swarm:
		{
			SpawnSwarmWave(simulation, waves.front());
		}
		break;

		default:
		{
			LOG_WARNING("[ServerSystems]::WaveSystem failed to initialize a wave spawn, [Controll the wave type]");
		}
		break;
		}

		//Add count and pop from queue
		waves.pop();
	}
}


/**Removes all enemies that has been destroyed and broadcasts the removal to the clients.
 *@param simulation	   Manages sending and removal of entities on the server.
 */
void ServerSystems::NextWaveConditions(Simulation* simulation, Timer& timer, int timeToFinish)
{
	//Publish event when timeToFinish been exceeded.
	if (simulation->m_timeCycler.GetSwitch())
	{
		simulation->GetGameScene()->publish<ESceneCallWaveSystem>(0.0f);
		simulation->m_timeCycler.Switch();
	}
}

void ServerSystems::UpdatePlayerWithInput(Simulation* simulation, HeadlessScene& scene, float dt, QuadTree* dynamicQT)
{
	scene.ForEachComponent<comp::Player, comp::Transform, comp::Velocity>([&](comp::Player& p, comp::Transform& t, comp::Velocity& v)
		{
			// update velocity
			sm::Vector3 vel = sm::Vector3(static_cast<float>(p.lastInputState.axisHorizontal), 0, static_cast<float>(p.lastInputState.axisVertical));
			vel.Normalize();

			sm::Vector3 cameraToPlayer = t.position - p.lastInputState.mouseRay.origin;
			cameraToPlayer.y = 0;
			cameraToPlayer.Normalize();
			float targetRotation = atan2(-cameraToPlayer.x, -cameraToPlayer.z);
			vel = sm::Vector3::TransformNormal(vel, sm::Matrix::CreateRotationY(targetRotation));

			if (vel.Length() > 0.01f)
				p.state = comp::Player::State::WALK;

			vel *= p.runSpeed;
			v.vel = vel;
		});

	scene.ForEachComponent<comp::Player, comp::AnimationState>([&](Entity e, comp::Player& p, comp::AnimationState& anim)
		{
			// Do stuff based on input

			// Get point on ground where mouse hovers over
			Plane_t plane;
			plane.normal = sm::Vector3(0, 1, 0);
			plane.point = sm::Vector3(0, 0, 0);

			if (!p.lastInputState.mouseRay.Intersects(plane, &p.mousePoint))
			{
				LOG_WARNING("Mouse click ray missed walking plane. Should not happen...");
			}

			if (p.state == comp::Player::State::WALK)
				anim.toSend = EAnimationType::MOVE;
			else
				anim.toSend = EAnimationType::IDLE;

			// check if using abilities
			if (p.lastInputState.leftMouse) // is held
			{
				p.state = comp::Player::State::LOOK_TO_MOUSE; // set state even if ability is not ready for use yet
				if (ecs::UseAbility(e, p.primaryAbilty, &p.mousePoint))
				{
					LOG_INFO("Used primary");
					anim.toSend = EAnimationType::PRIMARY_ATTACK;
				}

				// make sure movement alteration is not applied when using, because then its applied atomatically
				if (!ecs::IsUsing(e, p.primaryAbilty))
				{
					e.GetComponent<comp::Velocity>()->vel *= ecs::GetAbility(e, p.primaryAbilty)->movementSpeedAlt;
				}
			}
			else if (p.lastInputState.rightMouse)
			{
				if (ecs::UseAbility(e, p.secondaryAbilty, &p.mousePoint))
				{
					LOG_INFO("Used secondary");
					anim.toSend = EAnimationType::SECONDARY_ATTACK;
				}
			}

			if (p.lastInputState.key_r && simulation->m_timeCycler.GetTimePeriod() == Cycle::DAY) // was pressed
			{
				LOG_INFO("Pressed right");
				simulation->GetGrid().RemoveDefence(p.lastInputState.mouseRay, e.GetComponent<comp::Network>()->id, Blackboard::Get().GetPathFindManager());
			}

			if(p.lastInputState.key_shift)
			{
				if (ecs::UseAbility(e, p.moveAbilty, &p.mousePoint))
				{
					LOG_INFO("Used moveAbility");
					//anim.toSend = EAnimationType::MOVE_ABILITY;
				}
			}

			//Place defence on grid
			if (p.lastInputState.key_b && simulation->GetCurrency().GetAmount() >= 5 && simulation->m_timeCycler.GetTimePeriod() == Cycle::DAY)
			{
				if (simulation->GetGrid().PlaceDefence(p.lastInputState.mouseRay, e.GetComponent<comp::Network>()->id, Blackboard::Get().GetPathFindManager(), dynamicQT))
				{
					simulation->GetCurrency() -= 10;
					anim.toSend = EAnimationType::PLACE_DEFENCE;
				}
			}

			//Rotate defences 90 or not
			if (p.lastInputState.mousewheelDir != 0)
			{
				if (p.lastInputState.mousewheelDir > 0)
					p.rotateDefence = true;
				else if (p.lastInputState.mousewheelDir < 0)
					p.rotateDefence = false;
			}
		});


}

void ServerSystems::PlayerStateSystem(Simulation* simulation, HeadlessScene& scene, float dt)
{
	PROFILE_FUNCTION();
	scene.ForEachComponent<comp::Player>([](Entity e, comp::Player& p)
		{
			if (ecs::IsPlayerUsingAnyAbility(e))
			{
				p.state = comp::Player::State::LOOK_TO_MOUSE; // always use this state if any ability is being used
			}

		});

	scene.ForEachComponent<comp::Player, comp::Health, comp::Network>([&](Entity e, comp::Player& p, comp::Health& health, comp::Network n)
		{
			if (!health.isAlive)
			{
				//CHANGE TO DEAD ANIMATION?

				comp::Velocity* vel = e.GetComponent<comp::Velocity>();
				if (vel)
				{
					// dont move if dead
					vel->vel = sm::Vector3::Zero;
				}
				if (p.state != comp::Player::State::SPECTATING)
				{
					p.state = comp::Player::State::SPECTATING;
					message<GameMsg> msg;
					msg.header.id = GameMsg::Game_StartSpectate;
					
					simulation->SendMsg(n.id, msg);
				}
				p.respawnTimer -= dt;

				if (p.respawnTimer < 0.01f)
				{
					simulation->ResetPlayer(e);
					message<GameMsg> msg;
					msg.header.id = GameMsg::Game_StopSpectate;
					simulation->SendMsg(n.id, msg);
					LOG_INFO("Player %u respawned...", e.GetComponent<comp::Network>()->id);
				}
			}
		});

	scene.ForEachComponent<comp::Player, comp::Velocity, comp::Transform>([&](Entity e, comp::Player& p, comp::Velocity& v, comp::Transform& t)
		{
			if (p.state == comp::Player::State::LOOK_TO_MOUSE)
			{
				// turns player to look at mouse world pos
				p.fowardDir = p.mousePoint - t.position;
				p.fowardDir.Normalize();
			}
			else if (p.state == comp::Player::State::WALK)
			{
				// turns player with velocity
				sm::Vector3 dir;
				v.vel.Normalize(dir);
				p.fowardDir = dir;
			}
		});

	// if player is turning, turns until forward is reached
	scene.ForEachComponent<comp::Player, comp::Transform>([&](Entity e, comp::Player& p, comp::Transform& t)
		{
			if (p.state == comp::Player::State::WALK || p.state == comp::Player::State::LOOK_TO_MOUSE)
			{
				float time = dt * p.runSpeed * 0.4f;

				if (ecs::StepRotateTo(t.rotation, p.fowardDir, time))
				{
					p.state = comp::Player::State::IDLE;
				}
				e.UpdateNetwork();
			}
		});

	// turns npc with velocity
	scene.ForEachComponent<comp::NPC, comp::Transform, comp::Velocity>([&](Entity e, comp::NPC& p, comp::Transform& t, comp::Velocity& v)
		{
			if (v.vel.Length() > 0.001f)
			{
				float time = dt * p.movementSpeed * 0.5f;
				if (ecs::StepRotateTo(t.rotation, v.vel, time))
				{

				}
				e.UpdateNetwork();
			}
		});
}

void ServerSystems::CheckGameOver(Simulation* simulation, HeadlessScene& scene)
{
	PROFILE_FUNCTION();

	bool gameOver = true;

	//Check if all players is dead
	scene.ForEachComponent<comp::Player, comp::Health>([&](comp::Player& p, comp::Health& h)
		{
			if (h.isAlive)
			{
				gameOver = false;
			}
		});

	if (gameOver)
	{
		simulation->SetGameOver();
	}
}

void ServerSystems::TickBTSystem(Simulation* simulation, HeadlessScene& scene)
{
	scene.ForEachComponent<comp::BehaviorTree>([&](Entity entity, comp::BehaviorTree& bt)
		{
			bt.root->Tick();
		});
}

void ServerSystems::AnimatonSystem(Simulation* simulation, HeadlessScene& scene)
{
	scene.ForEachComponent<comp::Network, comp::AnimationState>([&](comp::Network& net, comp::AnimationState& anim)
		{
			//Have to send every time - otherwise animations can be locked to one
			if (anim.toSend != EAnimationType::NONE)
			{
				message<GameMsg>msg;
				msg.header.id = GameMsg::Game_ChangeAnimation;
				msg << anim.toSend << net.id;
				simulation->Broadcast(msg);

				anim.lastSend = anim.toSend;
				anim.toSend = EAnimationType::NONE;
			}
		});
}

void ServerSystems::DeathParticleTimer(HeadlessScene& scene)
{
	scene.ForEachComponent<comp::PARTICLEEMITTER>([&](Entity& e, comp::PARTICLEEMITTER& emitter)
		{
			if (emitter.hasDeathTimer == true && emitter.lifeLived <= emitter.lifeTime)
			{
				emitter.lifeLived += Stats::Get().GetFrameTime();
			}
			else if (emitter.hasDeathTimer == true && emitter.lifeLived >= emitter.lifeTime)
			{
				e.RemoveComponent<comp::PARTICLEEMITTER>();
			}
		});
}
