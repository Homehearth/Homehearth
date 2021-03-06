#include "NetServerPCH.h"
#include "Simulation.h"
#include "Wave.h"

void Simulation::InsertEntityIntoMessage(Entity entity, message<GameMsg>& msg, const std::bitset<ecs::Component::COMPONENT_MAX>& componentMask)const
{
	std::bitset<ecs::Component::COMPONENT_MAX> compSet;

	for (uint32_t i = 0; i < ecs::Component::COMPONENT_COUNT; i++)
	{
		if (!componentMask.test(i))
			continue;

		switch (i)
		{
		case ecs::Component::TRANSFORM:
		{
			comp::Transform* t = entity.GetComponent<comp::Transform>();
			if (t)
			{
				compSet.set(ecs::Component::TRANSFORM);
				msg << *t;
			}
			break;
		}
		case ecs::Component::VELOCITY:
		{
			comp::Velocity* v = entity.GetComponent<comp::Velocity>();
			if (v)
			{
				compSet.set(ecs::Component::VELOCITY);
				msg << *v;
			}
			break;
		}
		case ecs::Component::MESH_NAME:
		{
			comp::MeshName* m = entity.GetComponent<comp::MeshName>();
			if (m)
			{
				compSet.set(ecs::Component::MESH_NAME);
				msg << m->name;
			}

			break;
		}
		case ecs::Component::ANIMATOR_NAME:
		{
			comp::AnimatorName* m = entity.GetComponent<comp::AnimatorName>();
			if (m)
			{
				compSet.set(ecs::Component::ANIMATOR_NAME);
				msg << m->name;
			}

			break;
		}
		case ecs::Component::HEALTH:
		{
			comp::Health* h = entity.GetComponent<comp::Health>();
			if (h)
			{
				compSet.set(ecs::Component::HEALTH);
				msg << *h;
			}
			break;
		}
		case ecs::Component::BOUNDING_ORIENTED_BOX:
		{
			comp::OrientedBoxCollider* b = entity.GetComponent<comp::OrientedBoxCollider>();
			if (b)
			{
				dx::BoundingOrientedBox obb;
				obb.Center = b->Center;
				obb.Extents = b->Extents;
				obb.Orientation = b->Orientation;
				compSet.set(ecs::Component::BOUNDING_ORIENTED_BOX);
				msg << obb;
			}
			break;
		}
		case ecs::Component::BOUNDING_SPHERE:
		{
			comp::SphereCollider* bs = entity.GetComponent<comp::SphereCollider>();
			if (bs)
			{
				dx::BoundingSphere sphere;
				sphere.Center = bs->Center;
				sphere.Radius = bs->Radius;
				compSet.set(ecs::Component::BOUNDING_SPHERE);
				msg << sphere;
			}
			break;
		}
		case ecs::Component::PARTICLEMITTER:
		{
			comp::ParticleEmitter* p = entity.GetComponent<comp::ParticleEmitter>();
			if (p)
			{
				compSet.set(ecs::Component::PARTICLEMITTER);
				msg << *p;
			}
			break;
		}
		case ecs::Component::PLAYER:
		{
			comp::Player* p = entity.GetComponent<comp::Player>();
			if (p)
			{
				compSet.set(ecs::Component::PLAYER);
				msg << *p;
			}
			break;
		}
		case ecs::Component::COST:
		{
			comp::Cost* c = entity.GetComponent<comp::Cost>();
			if (c)
			{
				compSet.set(ecs::Component::COST);
				msg << *c;
			}
			break;
		}
		case ecs::Component::KD:
		{
			comp::KillDeaths* kd = entity.GetComponent<comp::KillDeaths>();
			if (kd)
			{
				compSet.set(ecs::Component::KD);
				msg << *kd;
			}
			break;
		}
		default:
			LOG_WARNING("Trying to send unimplemented component %u", i);
			break;
		}
	}

	msg << static_cast<uint32_t>(compSet.to_ulong());
	msg << entity.GetComponent<comp::Network>()->id;
}

void Simulation::ResetPlayer(Entity player)
{
	comp::Player* playerComp = player.GetComponent<comp::Player>();
	comp::KillDeaths* kd = player.AddComponent<comp::KillDeaths>();
	if (!playerComp)
	{
		LOG_WARNING("ResetPlayer: Entity is not a Player");
		return;
	}

	playerComp->wantsToSkipDay = false;
	playerComp->runSpeed = 30.f;
	playerComp->state = comp::Player::State::IDLE;
	playerComp->isReady = false;
	kd->kills = 0;
	kd->deaths = 0;

	comp::Transform* transform = player.AddComponent<comp::Transform>();
	transform->position = playerComp->spawnPoint;

	transform->scale = sm::Vector3(1.8f, 1.8f, 1.8f);

	player.AddComponent<comp::Velocity>();

	bool firstTimeAdded = false;
	if (!player.GetComponent<comp::AnimatorName>())
	{
		firstTimeAdded = true;
	}

	player.AddComponent<comp::AnimationState>();
	comp::Health* health = player.AddComponent<comp::Health>();
	health->isAlive = true;

	// only if Melee
	if (playerComp->classType == comp::Player::Class::WARRIOR)
	{
		player.RemoveComponent<comp::RangeAttackAbility>();
		player.RemoveComponent<comp::BlinkAbility>();
		player.RemoveComponent<comp::HealAbility>();

		health->maxHealth = 125.f;
		health->currentHealth = 125.f;

		comp::MeleeAttackAbility* attackAbility = player.AddComponent<comp::MeleeAttackAbility>();
		attackAbility->cooldown = 0.60f;
		attackAbility->attackDamage = 25.f;
		attackAbility->lifetime = 0.1f;
		attackAbility->useTime = 0.5f;
		attackAbility->delay = 0.2f;
		attackAbility->attackRange = 5.f;
		attackAbility->movementSpeedAlt = 0.5f;

		playerComp->primaryAbilty = entt::resolve<comp::MeleeAttackAbility>();

		comp::ShieldBlockAbility* block = player.AddComponent<comp::ShieldBlockAbility>();
		block->cooldown = 5.0f;
		block->delay = 0.0f;
		block->lifetime = 0.5f;
		block->movementSpeedAlt = 0.3f;
		block->useTime = 1000000.f;
		block->maxDamage = 70.f;

		playerComp->secondaryAbilty = entt::resolve<comp::ShieldBlockAbility>();


		comp::DashAbility* dashAbility = player.AddComponent<comp::DashAbility>();
		dashAbility->cooldown = 6.0f;
		dashAbility->delay = 0.0f;
		dashAbility->force = 7.f;
		dashAbility->lifetime = 0.0f;
		dashAbility->useTime = 0.20f;

		playerComp->moveAbilty = entt::resolve<comp::DashAbility>();

		player.AddComponent<comp::MeshName>()->name = NameType::MESH_KNIGHT;
		player.AddComponent<comp::AnimatorName>()->name = AnimName::ANIM_KNIGHT;
	}
	else if (playerComp->classType == comp::Player::Class::MAGE)
	{
		player.RemoveComponent<comp::MeleeAttackAbility>();
		player.RemoveComponent<comp::DashAbility>();
		player.RemoveComponent<comp::ShieldBlockAbility>();

		health->maxHealth = 80.f;
		health->currentHealth = 80.f;

		comp::RangeAttackAbility* attackAbility = player.AddComponent<comp::RangeAttackAbility>();
		attackAbility->cooldown = 1.2f;
		attackAbility->attackDamage = 15.f;
		attackAbility->lifetime = 0.1f;
		attackAbility->projectileSpeed = 80.f;
		attackAbility->projectileSize = 2.5f;
		attackAbility->attackRange = 13.0f;
		attackAbility->useTime = 0.4f;
		attackAbility->delay = 0.4f;
		attackAbility->movementSpeedAlt = 0.1f;
		playerComp->primaryAbilty = entt::resolve<comp::RangeAttackAbility>();

		comp::HealAbility* healAbility = player.AddComponent<comp::HealAbility>();
		healAbility->cooldown = 8.0f;
		healAbility->delay = 0.0f;
		healAbility->healAmount = 30.f;
		healAbility->lifetime = 1.0f;
		healAbility->range = 30.f;
		healAbility->useTime = 0.2f;
		healAbility->movementSpeedAlt = 1.0f;

		playerComp->secondaryAbilty = entt::resolve<comp::HealAbility>();

		comp::BlinkAbility* blinkAbility = player.AddComponent<comp::BlinkAbility>();
		blinkAbility->cooldown = 6.0f;
		blinkAbility->delay = 0.1f;
		blinkAbility->distance = 50.f;
		blinkAbility->lifetime = 0.1f;
		blinkAbility->useTime = 0.1f;

		playerComp->moveAbilty = entt::resolve<comp::BlinkAbility>();

		player.AddComponent<comp::MeshName>()->name = NameType::MESH_MAGE;
		player.AddComponent<comp::AnimatorName>()->name = AnimName::ANIM_MAGE;
	}

	player.AddComponent<comp::SphereCollider>()->Radius = 2.5f;

	//
	// AudioState
	//
	player.AddComponent<ecs::component::AudioState>();

	//Collision will handle this entity as a dynamic one
	player.AddComponent<comp::Tag<TagType::DYNAMIC>>();
	player.AddComponent<comp::Tag<TagType::GOOD>>(); // this is a good guy, he will call you back

	player.RemoveComponent<comp::TemporaryPhysics>();
	if (!firstTimeAdded)
	{
		m_pGameScene->publish<EComponentUpdated>(player, ecs::Component::MESH_NAME);
		m_pGameScene->publish<EComponentUpdated>(player, ecs::Component::ANIMATOR_NAME);
	}

	m_pGameScene->publish<EComponentUpdated>(player, ecs::Component::HEALTH);

	player.UpdateNetwork();
}

Simulation::Simulation(Server* pServer, HeadlessEngine* pEngine)
	: m_pServer(pServer)
	, m_pEngine(pEngine), m_pLobbyScene(nullptr), m_pGameScene(nullptr),
	m_pCurrentScene(nullptr), currentRound(0), m_shop(this)
{
	this->m_gameID = 0;
	this->m_tick = 0;
	this->m_wavesSurvived = 0;

	dx::BoundingBox bounds = { dx::XMFLOAT3(250, 0, -320), dx::XMFLOAT3(230, 50, 220) };
	qt = std::make_unique<QuadTree>(bounds);

	qtDynamic = std::make_unique<QuadTree>(bounds);

	m_spawnPoints.push(TL);
	m_spawnPoints.push(TR);
	m_spawnPoints.push(BL);
	m_spawnPoints.push(BR);
}

void Simulation::JoinLobby(uint32_t gameID, uint32_t playerID, const std::string& name)
{
	m_lobby.AddPlayer(gameID, playerID, name);
}

void Simulation::LeaveLobby(uint32_t playerID)
{
	this->SendRemoveAllEntitiesToPlayer(playerID);

	m_lobby.RemovePlayer(playerID);
	// Send to client the message with the new game ID
	message<GameMsg> accMsg;
	accMsg.header.id = GameMsg::Lobby_AcceptedLeave;


	m_pServer->SendToClient(playerID, accMsg);
}

bool Simulation::Create(uint32_t gameID, std::vector<dx::BoundingOrientedBox>* mapColliders, std::unordered_map<std::string, comp::OrientedBoxCollider>* houseColliders)
{
	this->m_gameID = gameID;
	this->m_lobby.Init(this);

	// Create Scenes associated with this Simulation
	m_pLobbyScene = &m_pEngine->GetScene("Lobby_" + std::to_string(gameID));

	m_pGameScene = &m_pEngine->GetScene("Game_" + std::to_string(gameID));

	houseManager.SetHouseColliders(houseColliders);

	m_pGameScene->on<ESceneUpdate>([&](const ESceneUpdate& e, HeadlessScene& scene)
		{
#if GOD_MODE
			/*
				Infinite health.
			*/
			scene.ForEachComponent<comp::Player, comp::Health>([](comp::Player& p, comp::Health& h) {

				h.currentHealth = h.maxHealth;

				});

#endif
			//  run all game logic systems
			{
				PROFILE_SCOPE("Systems");

				ServerSystems::CheckGameOver(this, scene);
				{
					PROFILE_SCOPE("Update QuadTree");
					Systems::UpdateDynamicQT(scene, qtDynamic.get());
				}
				{
					PROFILE_SCOPE("BT Tick");
					ServerSystems::TickBTSystem(this, scene);
				}
				{
					PROFILE_SCOPE("Input from Player");
					ServerSystems::UpdatePlayerWithInput(this, scene, e.dt, qt.get(), blackboard.get());
				}
				{
					PROFILE_SCOPE("Player state");
					ServerSystems::PlayerStateSystem(this, scene, e.dt);
				}

				{
					PROFILE_SCOPE("Abilities and Combat");
					Systems::UpdateAbilities(scene, e.dt);
					ServerSystems::CombatSystem(scene, e.dt, blackboard.get());
					Systems::HealingSystem(scene, e.dt);
					Systems::SelfDestructSystem(scene, e.dt);
				}

				{
					PROFILE_SCOPE("Animation Transform");
					Systems::TransformAnimationSystem(scene, e.dt);
				}

				{
					PROFILE_SCOPE("Movement collider system");
					Systems::MovementSystem(scene, e.dt);
					Systems::MovementColliderSystem(scene, e.dt);
				}

				{
					PROFILE_SCOPE("Fetch Collision List");
					Systems::FetchCollidingList(scene, qt.get(), qtDynamic.get());
				}
				{
					PROFILE_SCOPE("Collision");
					Systems::CheckCollisions(scene, e.dt);
				}

				ServerSystems::HealthSystem(scene, e.dt, m_currency, houseManager, qt.get(), m_grid, m_spreeHandler, blackboard.get());

				{
					PROFILE_SCOPE("Animation system");
					ServerSystems::AnimationSystem(this, scene);
				}
				{
					PROFILE_SCOPE("Clear quad tree");
					Systems::ClearCollidingList(scene, qtDynamic.get());
				}
				ServerSystems::SoundSystem(this, scene);
			}
			m_timeCycler.Update(e.dt, scene);

			{
				PROFILE_SCOPE("Hover defences");
				std::vector<Entity> updateEntities;
				if (m_timeCycler.GetTimePeriod() == CyclePeriod::NIGHT)
				{
					updateEntities = m_grid.HideHoverDefence();
				}
				else
				{
					updateEntities = m_grid.UpdateHoverDefence();
				}
				for (size_t i = 0; i < updateEntities.size(); i++)
				{
					m_updatedEntities.push_back(updateEntities.at(i));
				}
			}

			{
				PROFILE_SCOPE("Cycle changed conditions");
				ServerSystems::OnCycleChange(this);
			}

			ServerSystems::DeathParticleTimer(scene);

			m_spreeHandler.Update();

		});

	//On all enemies wiped, activate the next wave.
	m_pGameScene->on<ESceneCallWaveSystem>([&](const ESceneCallWaveSystem& dt, HeadlessScene& scene)
		{
			ServerSystems::WaveSystem(this, waveQueue);
		});

	m_pGameScene->GetRegistry()->on_construct<comp::Network>().connect<&Simulation::OnNetworkEntityCreate>(this);
	m_pGameScene->GetRegistry()->on_destroy<comp::Network>().connect<&Simulation::OnNetworkEntityDestroy>(this);
	m_pGameScene->GetRegistry()->on_update<comp::Network>().connect<&Simulation::OnNetworkEntityUpdated>(this);
	m_pGameScene->on<EComponentUpdated>([&](const EComponentUpdated& e, HeadlessScene& scene)
		{
			OnComponentUpdated(e.entity, e.component);
		});

	//Gridsystem
	m_addedEntities.clear();
	m_grid.Initialize(gridOptions.mapSize, gridOptions.position, gridOptions.fileName, m_pGameScene);
	//Create the nodes for AI handler on blackboard

#if RENDER_AINODES
	std::vector<std::vector<std::shared_ptr<Node>>> nodes = blackboard.GetPathFindManager()->GetNodes();
	for (int y = 0; y < nodes[0].size(); y++)
	{
		for (int x = 0; x < nodes[0].size(); x++)
		{
			if (nodes[y][x].get()->reachable)
			{
				Entity cube = m_pGameScene->CreateEntity();
				cube.AddComponent<comp::Transform>()->position = nodes[y][x].get()->position;
				cube.AddComponent<comp::MeshName>()->name = NameType::MESH_CUBE;
				cube.AddComponent<comp::Network>();
			}

		}
	}
#endif RENDER_AINODES
	m_removedEntities.clear();

	this->BuildMapColliders(mapColliders);
	EnemyManagement::CreateWaves(waveQueue, currentRound++);

	m_rainEntity = m_pGameScene->CreateEntity();
	m_pCurrentScene = m_pLobbyScene;

	return true;
}

void Simulation::Destroy()
{
	m_pGameScene->GetRegistry()->on_construct<comp::Network>().disconnect<&Simulation::OnNetworkEntityCreate>(this);
	m_pGameScene->GetRegistry()->on_destroy<comp::Network>().disconnect<&Simulation::OnNetworkEntityDestroy>(this);
	m_pGameScene->Clear();
	m_pLobbyScene->Clear();
}

void Simulation::SendAddedEntities()
{
	// All new Entities
	this->SendEntities(m_addedEntities, GameMsg::Game_AddEntity);
	m_addedEntities.clear();
}

void Simulation::SendSnapshot()
{
	PROFILE_FUNCTION();

	if (m_pCurrentScene == m_pGameScene)
	{
		if (!m_updatedComponents.empty())
		{
			for (auto& pair : m_updatedComponents)
			{
				std::bitset<ecs::Component::COMPONENT_MAX> compMask;
				compMask.set(pair.first);
				this->SendEntities(pair.second, GameMsg::Game_UpdateComponent, compMask);
			}
			m_updatedComponents.clear();
		}

		std::bitset<ecs::Component::COMPONENT_MAX> compMask;
		compMask.set(ecs::Component::TRANSFORM);
		compMask.set(ecs::Component::BOUNDING_ORIENTED_BOX);
#if DEBUG_SNAPSHOT
		compMask.set(ecs::Component::BOUNDING_SPHERE);
#endif

		this->SendEntities(m_updatedEntities, GameMsg::Game_Snapshot, compMask);
		m_updatedEntities.clear();

		if (m_currency.m_hasUpdated)
		{
			network::message<GameMsg> msg3;
			msg3.header.id = GameMsg::Game_Money;
			msg3 << m_currency.GetAmount();
			this->Broadcast(msg3);
			m_currency.m_hasUpdated = false;
		}
		//Send Abilities
		{
			for (auto i = m_lobby.m_players.begin(); i != m_lobby.m_players.end(); i++)
			{
				network::message<GameMsg> msg4;
				msg4.header.id = GameMsg::Game_Cooldown;
				Entity p = i->second;
				uint32_t count = 0;
				comp::MeleeAttackAbility* melee = p.GetComponent<comp::MeleeAttackAbility>();
				comp::RangeAttackAbility* range = p.GetComponent<comp::RangeAttackAbility>();
				comp::BlinkAbility* blink = p.GetComponent<comp::BlinkAbility>();
				comp::DashAbility* dash = p.GetComponent<comp::DashAbility>();
				comp::HealAbility* heal = p.GetComponent<comp::HealAbility>();
				comp::ShieldBlockAbility* block = p.GetComponent<comp::ShieldBlockAbility>();
				if (melee)
				{
					count++;
					msg4 << AbilityIndex::Primary << melee->cooldownTimer << melee->cooldown;
				}
				else if (range)
				{
					count++;
					msg4 << AbilityIndex::Primary << range->cooldownTimer << range->cooldown;
				}

				if (blink)
				{
					count++;
					msg4 << AbilityIndex::Dodge << blink->cooldownTimer << blink->cooldown;
				}
				else if (dash)
				{
					count++;
					msg4 << AbilityIndex::Dodge << dash->cooldownTimer << dash->cooldown;
				}

				if (heal)
				{
					count++;
					msg4 << AbilityIndex::Secondary << heal->cooldownTimer << heal->cooldown;
				}

				if (block)
				{
					count++;
					msg4 << AbilityIndex::Secondary << block->cooldownTimer << block->cooldown;
				}

				msg4 << count;
				msg4 << msg4.header.id;
				m_pServer->SendToClientUDP(i->first, msg4);
			}
		}

		//network::message<GameMsg> msg5;
		//msg5.header.id = GameMsg::Game_Spree;
		//msg5 << (uint32_t)m_spreeHandler.GetSpree();
		//this->Broadcast(msg5);

		if (this->m_tick % 600 == 0)
		{
			network::message<GameMsg> timeMsg;
			timeMsg.header.id = GameMsg::Game_Time_Update;
			timeMsg << m_timeCycler.GetCycleSpeed();
			timeMsg << m_timeCycler.GetTime();
			this->Broadcast(timeMsg);
		}
	}
	else
	{
		m_lobby.Update();
	}
}

void Simulation::Update(float dt)
{
	PROFILE_FUNCTION();

	if (!m_lobby.IsEmpty())
	{
		// ORDER IS IMPORTANT
		m_lobby.ScanForDisconnects();

		this->SendAddedEntities();

		m_pCurrentScene->Update(dt);

		this->SendRemovedEntities();

		this->SendSnapshot();

		m_tick++;
	}
}

void Simulation::UpdateInput(InputState state, uint32_t playerID)
{
	if (m_pCurrentScene != m_pGameScene)
		return;

	Entity e = m_lobby.GetPlayer(playerID);
	if (!e.IsNull() && e.GetComponent<comp::Health>()->isAlive)
	{
		comp::Player* p = e.GetComponent<comp::Player>();

		p->lastInputState = p->inputState;
		p->inputState = state;
	}
}

void Simulation::OnNetworkEntityCreate(entt::registry& reg, entt::entity entity)
{
	Entity e(reg, entity);
	// Network has surely been added
	comp::Network* net = e.GetComponent<comp::Network>();
	if (net->id == UINT32_MAX)
	{
		net->id = m_pServer->PopNextUniqueID();
	}
	m_addedEntities.push_back(e);
}

void Simulation::OnNetworkEntityDestroy(entt::registry& reg, entt::entity entity)
{
	Entity e(reg, entity);
	// Network has not been destroyed yet
	comp::Network* net = e.GetComponent<comp::Network>();
	m_removedEntities.push_back(net->id);
}

void Simulation::OnNetworkEntityUpdated(entt::registry& reg, entt::entity entity)
{
	Entity e(reg, entity);

	if (!e.IsNull())
	{
		if (std::find(m_updatedEntities.begin(), m_updatedEntities.end(), e) == m_updatedEntities.end())
		{
			m_updatedEntities.push_back(e);
		}
	}
}
void Simulation::OnComponentUpdated(Entity entity, ecs::Component component)
{
	if (entity.GetComponent<comp::Network>())
	{
		if (std::find(m_updatedComponents[component].begin(), m_updatedComponents[component].end(), entity) == m_updatedComponents[component].end())
		{
			m_updatedComponents[component].push_back(entity);
		}
	}
}

void Simulation::BuildMapColliders(std::vector<dx::BoundingOrientedBox>* mapColliders)
{
	for (size_t i = 0; i < mapColliders->size(); i++)
	{
		Entity collider = m_pGameScene->CreateEntity();
		comp::OrientedBoxCollider* obb = collider.AddComponent<comp::OrientedBoxCollider>();
		obb->Center = mapColliders->at(i).Center;
		obb->Extents = mapColliders->at(i).Extents;
		obb->Orientation = mapColliders->at(i).Orientation;
		collider.AddComponent<comp::Tag<STATIC>>();
		// Map bounds is loaded in first, 6 obbs surrounding village put the correct tag for collision system
		if (i < 6)
		{
			collider.AddComponent<comp::Tag<MAP_BOUNDS>>();
		}
#if RENDER_COLLIDERS
		collider.AddComponent<comp::Network>();
#endif
		qt->Insert(collider);
	}
}

HeadlessScene* Simulation::GetLobbyScene() const
{
	return m_pLobbyScene;
}

HeadlessScene* Simulation::GetGameScene() const
{
	return m_pGameScene;
}

void Simulation::SetScene(HeadlessScene* scene)
{
	if (!scene)
	{
		m_pCurrentScene = scene;
	}
}

GridSystem& Simulation::GetGrid()
{
	return m_grid;
}

Currency& Simulation::GetCurrency()
{
	return m_currency;
}

void Simulation::UpgradeDefence(const uint32_t& id)
{
	m_pCurrentScene->ForEachComponent<comp::Network>([&](Entity e, comp::Network& n)
		{
			if (n.id == id)
			{
				comp::Cost* c = e.GetComponent<comp::Cost>();
				comp::Health* h = e.GetComponent<comp::Health>();
				comp::MeshName* m = e.GetComponent<comp::MeshName>();

				if (c && h && m)
				{
					if (m_currency >= c->cost && h->upgradeLevel <= 2)
					{
						// Add upgrades here.
						h->maxHealth += 35;
						h->currentHealth += 35;
						h->upgradeLevel++;

						switch (m->name)
						{
						case NameType::MESH_DEFENCE1X1_LVL0:
						{
							m->name = NameType::MESH_DEFENCE1X1_LVL1;
							break;
						}
						case NameType::MESH_DEFENCE1X1_LVL1:
						{
							m->name = NameType::MESH_DEFENCE1X1_LVL2;
							break;
						}
						case NameType::MESH_DEFENCE1X1_LVL2:
						{
							m->name = NameType::MESH_DEFENCE1X1_LVL3;
							break;
						}
						case NameType::MESH_DEFENCE1X3_LVL0:
						{
							m->name = NameType::MESH_DEFENCE1X3_LVL1;
							break;
						}
						case NameType::MESH_DEFENCE1X3_LVL1:
						{
							m->name = NameType::MESH_DEFENCE1X3_LVL2;
							break;
						}
						case NameType::MESH_DEFENCE1X3_LVL2:
						{
							m->name = NameType::MESH_DEFENCE1X3_LVL3;
							break;
						}
						default:
							break;
						}

						// Cost is here.
						e.UpdateNetwork();
						m_currency -= c->cost;
						c->cost *= static_cast<uint32_t>(1.5f);
						m_pGameScene->publish<EComponentUpdated>(e, ecs::Component::COST);
						m_pGameScene->publish<EComponentUpdated>(e, ecs::Component::MESH_NAME);
					}
				}
			}
		});
}

void Simulation::ClearOutgoing()
{
	m_addedEntities.clear();
	m_removedEntities.clear();
	m_updatedComponents.clear();
	m_updatedEntities.clear();
}

void Simulation::SetLobbyScene()
{
	m_pCurrentScene = m_pLobbyScene;
	m_lobby.SetActive(true);
}

void Simulation::SetGameScene()
{
	ResetGameScene();
	m_pCurrentScene = m_pGameScene;
	m_lobby.SetActive(false);
#if GOD_MODE
	// During debug give players 1000 gold/monies.
	m_currency = 10000;
#else
	m_currency = 500;
#endif

#if NO_CLIP
	for (auto& player : m_lobby.m_players)
	{
		player.second.RemoveComponent<comp::Tag<TagType::GOOD>>();
	}
#endif
}

void Simulation::ResetGameScene()
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_RemoveEntity;
	uint32_t count = 0;
	currentRound = 0;

	while (!m_spawnPoints.empty())
	{
		m_spawnPoints.pop();
	}

	m_spawnPoints.push(TL);
	m_spawnPoints.push(TR);
	m_spawnPoints.push(BL);
	m_spawnPoints.push(BR);

	this->m_pGameScene->ForEachComponent<comp::Network>([&](Entity e, comp::Network& n)
		{
			if (!e.GetComponent<comp::Player>())
			{
				msg << n.id;
				count++;
				e.Destroy();
			}
			else
			{
				ResetPlayer(e);
			}
		});

	if (count > 0)
	{
		msg << count;
		this->Broadcast(msg);
	}

	m_currency = 500;

	EnemyManagement::CreateWaves(waveQueue, currentRound);
	blackboard = std::make_unique<Blackboard>();
	blackboard->GetPathFindManager()->CreateNodes(&m_grid);
	houseManager.SetBlackboard(blackboard.get());
	m_timeCycler.setBlackboard(blackboard.get());

	houseManager.InitializeHouses(*this->GetGameScene(), qt.get());
	AIBehaviors::UpdateBlackBoard(*m_pGameScene, blackboard.get());
	qt->ClearNullEntities();

#if DAYCYCLE
	m_timeCycler.SetTime(MORNING);
	m_timeCycler.SetCycleSpeed(1.0f);
#else
	m_timeCycler.SetTime(DAY);
	m_timeCycler.SetCycleSpeed(0.0f);
#endif
	m_tick = 0;
}

void Simulation::SendEntities(const std::vector<Entity>& entities, GameMsg msgID, const std::bitset<ecs::Component::COMPONENT_MAX>& componentMask)
{
	if (entities.size() == 0)
		return;

	uint32_t count = 0;
	message<GameMsg> msg;
	msg.header.id = msgID;

	for (size_t i = 0; i < entities.size(); i++)
	{
		if (!entities[i].IsNull())
		{
			InsertEntityIntoMessage(entities[i], msg, componentMask);
			count++;
		}

		if (count == PACKET_CHUNK_SIZE || i == entities.size() - 1)
		{
			msg << count;
			if (msgID == GameMsg::Game_Snapshot)
			{
				this->BroadcastUDP(msg);
			}
			else
			{
				this->Broadcast(msg);
			}
			msg.clear();
			count = 0;
		}
	}
}

void Simulation::SendAllEntitiesToPlayer(uint32_t playerID) const
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_AddEntity;
	uint32_t count = 0;
	m_pGameScene->ForEachComponent<comp::Network>([&](Entity e, comp::Network& n)
		{
			InsertEntityIntoMessage(e, msg);
			count++;

			if (count == PACKET_CHUNK_SIZE)
			{
				msg << count;

				this->m_pServer->SendToClient(playerID, msg);
				msg.clear();
				count = 0;
			}
		});

	if (count > 0)
	{
		msg << count;

		this->m_pServer->SendToClient(playerID, msg);
	}
}

void Simulation::SendRemoveAllEntitiesToPlayer(uint32_t playerID) const
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_RemoveEntity;
	uint32_t count = 0;
	m_pGameScene->ForEachComponent<comp::Network>([&](Entity e, comp::Network& n)
		{
			msg << n.id;
			count++;

			if (count == PACKET_CHUNK_SIZE)
			{
				msg << count;

				this->m_pServer->SendToClient(playerID, msg);
				msg.clear();
				count = 0;
			}
		});

	if (count > 0)
	{
		msg << count;

		this->m_pServer->SendToClient(playerID, msg);
	}
}

void Simulation::SendRemovedEntities()
{
	SendRemoveEntities(m_removedEntities);
}

void Simulation::SendRemoveEntities(const std::vector<uint32_t> entitiesNetIDs)
{
	if (entitiesNetIDs.size() == 0)
		return;

	uint32_t count = 0;
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_RemoveEntity;

	for (size_t i = 0; i < entitiesNetIDs.size(); i++)
	{
		msg << entitiesNetIDs[i];
		count++;
		if (count == PACKET_CHUNK_SIZE || i == entitiesNetIDs.size() - 1)
		{
			msg << count;
			this->Broadcast(msg);
			count = 0;
			msg.clear();
		}
	}

	m_removedEntities.clear();
}

void Simulation::SendMsg(uint32_t playerID, message<GameMsg>& msg) const
{
	this->m_pServer->SendToClient(playerID, msg);
}

void Simulation::SendMsgUDP(uint32_t playerID, message<GameMsg>& msg) const
{
	this->m_pServer->SendToClientUDP(playerID, msg);
}

bool Simulation::IsPlayerConnected(uint32_t playerID)
{
	return m_pServer->isClientConnected(playerID);
}

bool Simulation::IsEmpty() const
{
	return m_lobby.IsEmpty();
}

void Simulation::ReadyCheck(uint32_t playerID)
{
	bool isAllReady = m_lobby.ReadyCheck(playerID);
	m_lobby.Update();

	if (isAllReady)
	{
		// Set all players spawn positions once when everyone is ready.
		m_pGameScene->ForEachComponent<comp::Player>([&](comp::Player& p)
			{
				p.spawnPoint = m_spawnPoints.front();
				m_spawnPoints.pop();
			});
		// Start the game.
		SetGameScene();

		network::message<GameMsg> msg;
		msg.header.id = GameMsg::Game_Start;

		this->Broadcast(msg);
	}
}

void Simulation::Broadcast(message<GameMsg>& msg, uint32_t exclude) const
{
	auto it = m_lobby.m_players.begin();

	while (it != m_lobby.m_players.end())
	{
		if (exclude != it->first)
		{
			m_pServer->SendToClient(it->first, msg);
		}
		it++;
	}
}

void Simulation::BroadcastUDP(message<GameMsg>& msg, uint32_t exclude) const
{
	auto it = m_lobby.m_players.begin();

	msg << msg.header.id;

	while (it != m_lobby.m_players.end())
	{
		if (exclude != it->first)
		{
			m_pServer->SendToClientUDP(it->first, msg);
		}
		it++;
	}
}

Entity Simulation::GetPlayer(uint32_t playerID) const
{
	return m_lobby.GetPlayer(playerID);
}

Blackboard* Simulation::GetBlackboard()
{
	return blackboard.get();
}