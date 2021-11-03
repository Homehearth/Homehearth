#include "NetServerPCH.h"
#include "Simulation.h"

#include "Wave.h"

void Simulation::InsertEntityIntoMessage(Entity entity, message<GameMsg>& msg)const
{
	std::bitset<ecs::Component::COMPONENT_MAX> compSet;

	for (uint32_t i = 0; i < ecs::Component::COMPONENT_COUNT; i++)
	{
		switch (i)
		{
		case ecs::Component::NETWORK:
		{
			compSet.set(ecs::Component::NETWORK);
			msg << *entity.GetComponent<comp::Network>();
			break;
		}
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
		case ecs::Component::NAME_PLATE:
		{
			comp::NamePlate* n = entity.GetComponent<comp::NamePlate>();
			if (n)
			{
				compSet.set(ecs::Component::NAME_PLATE);
				msg << n->namePlate;
			}
			break;
		}
		case ecs::Component::HEALTH:
		{
			comp::Health* h = entity.GetComponent<comp::Health>();
			if (h)
			{
				compSet.set(ecs::Component::HEALTH);
				msg << h->currentHealth << h->isAlive << h->maxHealth;
			}
			break;
		}
		case ecs::Component::BOUNDING_ORIENTED_BOX:
		{
			comp::BoundingOrientedBox* b = entity.GetComponent<comp::BoundingOrientedBox>();
			if (b)
			{
				compSet.set(ecs::Component::BOUNDING_ORIENTED_BOX);
				msg << *b;
			}
			break;
		}
		case ecs::Component::BOUNDING_SPHERE:
		{
			comp::BoundingSphere* bs = entity.GetComponent<comp::BoundingSphere>();
			if (bs)
			{
				compSet.set(ecs::Component::BOUNDING_SPHERE);
				msg << *bs;
			}
			break;
		}
		case ecs::Component::LIGHT:
		{
			comp::Light* l = entity.GetComponent<comp::Light>();
			if (l)
			{
				compSet.set(ecs::Component::LIGHT);
				msg << *l;
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
		case ecs::Component::TILE:
		{
			comp::Tile* t = entity.GetComponent<comp::Tile>();
			if (t)
			{
				compSet.set(ecs::Component::TILE);
				msg << *t;
			}
			break;
		}
		default:
			LOG_WARNING("Trying to send unimplemented component %u", i);
			break;
		}
	}

	msg << static_cast<uint32_t>(compSet.to_ulong());
}

message<GameMsg> Simulation::AllEntitiesMessage()const
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_AddEntity;
	uint32_t count = 0;
	m_pGameScene->ForEachComponent<comp::Network>([&](Entity e, comp::Network& n)
		{
			InsertEntityIntoMessage(e, msg);
			count++;
		});
	msg << count;
	return msg;
}

void Simulation::CreateWaves()
{
	using namespace EnemyManagement;

	Wave wave1, wave2; // Default: WaveType::Zone
	{ // Wave_1 Group_1
		Wave::Group group1;
		group1.AddEnemy(EnemyType::Default, 50);
		group1.AddEnemy(EnemyType::Default2, 50);
		group1.SetSpawnPoint({ 200.f, 0.0f });
		wave1.SetTimeLimit(5);
		wave1.AddGroup(group1);
	}

	{ // Wave_1 Group_2
		Wave::Group group2;
		group2.AddEnemy(EnemyType::Default, 200);
		group2.SetSpawnPoint({ -200.f, 0.0f });
		wave1.AddGroup(group2);
	}
	waveQueue.emplace(wave1); // Add Wave_1

	{ // Wave_2 Group_3
		Wave::Group group3;
		group3.AddEnemy(EnemyType::Default, 5);
		group3.AddEnemy(EnemyType::Default2, 5);
		group3.SetSpawnPoint({ 0.f, -200.0f });
		wave2.AddGroup(group3);
	}

	{ // Wave_2 Group_4
		Wave::Group group4;
		group4.AddEnemy(EnemyType::Default, 2);
		group4.SetSpawnPoint({ 0.f, 200.0f });
		wave2.AddGroup(group4);
	}
	waveQueue.emplace(wave2); // Add Wave_2
}

Simulation::Simulation(Server* pServer, HeadlessEngine* pEngine)
	: m_pServer(pServer)
	, m_pEngine(pEngine), m_pLobbyScene(nullptr), m_pGameScene(nullptr), m_pCurrentScene(nullptr)
{
	this->m_gameID = 0;
	this->m_tick = 0;
}

bool Simulation::JoinLobby(uint32_t playerID, uint32_t gameID, const std::string& namePlate)
{
	if (m_pCurrentScene == m_pGameScene)
	{
		message<GameMsg> msg;
		msg.header.id = GameMsg::Lobby_Invalid;
		msg << std::string("Request denied: Game has already started!");

		m_pServer->SendToClient(playerID, msg);

		return false;
	}
	// Send to client the message with the new game ID
	if (m_players.size() < MAX_PLAYERS_PER_LOBBY)
	{
		message<GameMsg> msg;
		msg.header.id = GameMsg::Lobby_Accepted;
		msg << gameID;

		m_pServer->SendToClient(playerID, msg);

		// Add the players to the simulation on that specific client
		this->AddPlayer(playerID, namePlate);
	}
	else
	{
		network::message<GameMsg> msg;
		msg.header.id = GameMsg::Lobby_Invalid;
		std::string fullLobby = "Request denied: This lobby was full!";
		msg << fullLobby;
		m_pServer->SendToClient(playerID, msg);

		return false;
	}

	LOG_INFO("Player %d joined lobby: %d", playerID, gameID);
	return true;
}

bool Simulation::LeaveLobby(uint32_t playerID, uint32_t gameID)
{
	if (!this->RemovePlayer(playerID))
	{
		return false;
	}

	// Send to client the message with the new game ID
	message<GameMsg> accMsg;
	accMsg.header.id = GameMsg::Lobby_AcceptedLeave;

	m_pServer->SendToClient(playerID, accMsg);

	return true;
}

void Simulation::UpdateLobby()
{
	network::message<GameMsg> msg;
	msg.header.id = GameMsg::Lobby_Update;

	for (auto& player : m_players)
	{
		// Send ID first.
		msg << (uint32_t)player.first;
		// After that name.
		msg << player.second.GetComponent<comp::NamePlate>()->namePlate;
	}
	msg << (uint32_t)m_players.size();

	// Send message to each other player and skip the current one.
	this->Broadcast(msg);
}


bool Simulation::Create(uint32_t playerID, uint32_t gameID, std::vector<dx::BoundingOrientedBox>* mapColliders, const std::string& namePlate)
{
	this->m_gameID = gameID;

	// Create and add all waves to the queue.
	CreateWaves();

	// Create Scenes associated with this Simulation
	m_pLobbyScene = &m_pEngine->GetScene("Lobby_" + std::to_string(gameID));
	m_pLobbyScene->on<ESceneUpdate>([&](const ESceneUpdate& e, HeadlessScene& scene)
		{
			//LOG_INFO("LOBBY Scene %d", gameID);

		});

	m_pGameScene = &m_pEngine->GetScene("Game_" + std::to_string(gameID));
	m_pGameScene->on<ESceneUpdate>([&](const ESceneUpdate& e, HeadlessScene& scene)
		{
			// update components with input
			for (const auto& pair : m_playerInputs)
			{
				Entity e = pair.first;
				InputState input = pair.second;
				// update velocity
				e.GetComponent<comp::Velocity>()->vel = sm::Vector3(static_cast<float>(input.axisHorizontal), 0, static_cast<float>(input.axisVertical)) * e.GetComponent<comp::Player>()->runSpeed;

				// check if attacking
				if (input.leftMouse)
				{
					comp::CombatStats* stats = e.GetComponent<comp::CombatStats>();
					if (stats)
					{
						if (stats->cooldownTimer <= 0.0f)
							stats->isAttacking = true;

						stats->targetRay = input.mouseRay;

					}
					comp::Player* player = e.GetComponent<comp::Player>();
					if (player)
					{
						player->state = comp::Player::State::ATTACK;

					}
				}
			}

			//  run all game logic systems
			{
				PROFILE_SCOPE("Systems");
				Systems::CharacterMovement(scene, e.dt);
				Systems::MovementSystem(scene, e.dt);
				Systems::MovementColliderSystem(scene, e.dt);
				{
					PROFILE_SCOPE("Collision Box/Box");
					Systems::CheckCollisions<comp::BoundingOrientedBox, comp::BoundingOrientedBox>(scene, e.dt);
				}
				{
					PROFILE_SCOPE("Collision Box/Sphere");
					Systems::CheckCollisions<comp::BoundingOrientedBox, comp::BoundingSphere>(scene, e.dt);
				}
				Systems::CombatSystem(scene, e.dt);

			}

			if(!waveQueue.empty())
				ServerSystems::NextWaveConditions(this, waveTimer, waveQueue.front().GetTimeLimit());

			//LOG_INFO("GAME Scene %d", m_gameID);
		});

	//On all enemies wiped, activate the next wave.
	m_pGameScene->on<ESceneCallWaveSystem>([&](const ESceneCallWaveSystem& dt, HeadlessScene& scene)
		{
			waveTimer.Start();
			ServerSystems::WaveSystem(this, waveQueue);
		
		});
	
	//On collision event add entities as pair in the collision system
	m_pGameScene->on<ESceneCollision>([&](const ESceneCollision& e, HeadlessScene& scene)
		{
			CollisionSystem::Get().AddPair(e.obj1, e.obj2);
			CollisionSystem::Get().OnCollision(e.obj1, e.obj2);
		});

	m_pGameScene->GetRegistry()->on_construct<comp::Network>().connect<&Simulation::OnNetworkEntityCreate>(this);
	m_pGameScene->GetRegistry()->on_destroy<comp::Network>().connect<&Simulation::OnNetworkEntityDestroy>(this);
	m_pGameScene->GetRegistry()->on_update<comp::Network>().connect<&Simulation::OnNetworkEntityUpdated>(this);


	// ---DEBUG ENTITY---
	Entity e = m_pGameScene->CreateEntity();
	e.AddComponent<comp::Transform>()->position = sm::Vector3(-5, 0, 0);
	e.AddComponent<comp::MeshName>()->name = "Chest.obj";
	e.AddComponent<comp::BoundingOrientedBox>()->Extents = sm::Vector3(2.f, 2.f, 2.f);
	e.AddComponent<comp::Enemy>();
	e.AddComponent<comp::Health>();
	*e.AddComponent<comp::CombatStats>() = { 1.0f, 20.f, 1.0f, false, false };
	e.AddComponent<comp::Tag<TagType::STATIC>>();
	// send entity
	e.AddComponent<comp::Network>();

	// ---END OF DEBUG---

	// --- WORLD ---
	Entity e2 = m_pGameScene->CreateEntity();
	e2.AddComponent<comp::Transform>();// ->position = { -250, -2, 300 };
	e2.AddComponent<comp::MeshName>()->name = "GameScene.obj";
	e2.AddComponent<comp::Tag<TagType::STATIC>>();
	// send entity
	e2.AddComponent<comp::Network>();

	// --- END OF THE WORLD ---
	Entity collider;
	for (size_t i = 0; i < mapColliders->size(); i++)
	{
		collider = m_pGameScene->CreateEntity();
		collider.AddComponent<comp::BoundingOrientedBox>()->Center = mapColliders->at(i).Center;
		collider.GetComponent<comp::BoundingOrientedBox>()->Extents = mapColliders->at(i).Extents;
		collider.GetComponent<comp::BoundingOrientedBox>()->Orientation = mapColliders->at(i).Orientation;
		//collider.AddComponent<comp::Transform>()->position = mapColliders->at(i).Center;
		collider.AddComponent<comp::Network>();
		collider.AddComponent<comp::Tag<TagType::STATIC>>();
	}
	
	//Gridsystem
	GridProperties_t gridOption;
	m_grid.Initialize(gridOption.mapSize, gridOption.position, gridOption.fileName, m_pGameScene);
	
	m_addedEntities.clear();
	m_removedEntities.clear();

	m_pCurrentScene = m_pLobbyScene;


	// Automatically join created lobby
	JoinLobby(playerID, gameID, namePlate);


	return true;
}

void Simulation::Destroy()
{
	m_pGameScene->GetRegistry()->on_construct<comp::Network>().disconnect<&Simulation::OnNetworkEntityCreate>(this);
	m_pGameScene->GetRegistry()->on_destroy<comp::Network>().disconnect<&Simulation::OnNetworkEntityDestroy>(this);

	m_pGameScene->Clear();
	m_pLobbyScene->Clear();
}

void Simulation::ReadyCheck(const uint32_t& playerID)
{
	if (m_pCurrentScene == m_pLobbyScene)
	{
		if (m_players.find(playerID) != m_players.end())
		{
			m_players.at(playerID).GetComponent<comp::Player>()->isReady = true;

			// DEBUG
//#ifdef _DEBUG
			// Debugging allow only one player to start.
			m_pCurrentScene = m_pGameScene;
			// Start the game.
			network::message<GameMsg> msg;
			msg.header.id = GameMsg::Game_Start;
			this->Broadcast(msg);
			return;
			//#endif
			auto it = m_players.begin();

			uint32_t readyCount = 0;
			while (it != m_players.end())
			{
				if (it->second.GetComponent<comp::Player>()->isReady)
				{
					readyCount++;
				}
				it++;
			}

			// Start game when all players are marked ready
			if (readyCount == MAX_PLAYERS_PER_LOBBY)
			{
				m_pCurrentScene = m_pGameScene;
				// Start the game.
				network::message<GameMsg> msg;
				msg.header.id = GameMsg::Game_Start;
				this->Broadcast(msg);
			}
		}
	}
}

bool Simulation::IsEmpty() const
{
	return m_players.empty();
}

bool Simulation::AddPlayer(uint32_t playerID, const std::string& namePlate)
{
	if (!m_pServer->isClientConnected(playerID))
	{
		return false;
	}

	// Send all entities in Game Scene to new player
	this->SendAllEntitiesToPlayer(playerID);

	// Create Player entity in Game scene
	Entity player = m_pGameScene->CreateEntity();
	player.AddComponent<comp::Transform>()->position = sm::Vector3(320.f, 0, -310.f);
	player.AddComponent<comp::Velocity>();
	player.AddComponent<comp::NamePlate>()->namePlate = namePlate;
	player.AddComponent<comp::MeshName>()->name = "Arrow.fbx";
#ifdef _DEBUG
	player.AddComponent<comp::Player>()->runSpeed = 25.f;
#else
	player.AddComponent<comp::Player>()->runSpeed = 10.f;
#endif // _DEBUG

	*player.AddComponent<comp::CombatStats>() = { 0.3f, 20.f, 2.0f, true, 30.f};
	player.AddComponent<comp::Health>();
	player.AddComponent<comp::BoundingOrientedBox>();

	//Collision will handle this entity as a dynamic one
	player.AddComponent<comp::Tag<TagType::DYNAMIC>>();
	// Network component will make sure the new entity is sent
	player.AddComponent<comp::Network>(playerID);

	m_players[playerID] = player;

	return true;
}

bool Simulation::AddEnemy()
{
	// Create Enemy entity in Game scene.
	Entity enemy = m_pGameScene->CreateEntity();
	enemy.AddComponent<comp::Transform>();
	const unsigned char BAD = 8;
	enemy.AddComponent<comp::Tag<BAD>>();
	enemy.AddComponent<comp::Health>();
	enemy.AddComponent<comp::Network>();

	return true;
}

bool Simulation::RemovePlayer(uint32_t playerID)
{
	this->SendRemoveAllEntitiesToPlayer(playerID);

	Entity player = m_players.at(playerID);
	if (m_playerInputs.find(player) != m_playerInputs.end())
	{
		m_playerInputs.erase(player);
	}
	m_players.erase(playerID);

	if (!player.Destroy())
	{
		LOG_INFO("Player %u entity could not be removed", playerID);
		return false;
	}
	LOG_INFO("Removed player %u from scene", playerID);

	return true;
}

std::unordered_map<uint32_t, Entity>::iterator Simulation::RemovePlayer(std::unordered_map<uint32_t, Entity>::iterator playerIterator)
{
	Entity player = playerIterator->second;
	uint32_t playerID = playerIterator->first;
	if (m_playerInputs.find(player) != m_playerInputs.end())
	{
		m_playerInputs.erase(player);
	}
	auto it = m_players.erase(playerIterator);

	if (!player.Destroy())
	{
		LOG_INFO("Player %u entity could not be removed", playerID);
	}
	else
	{
		LOG_INFO("Removed player %u from scene", playerID);
	}

	return it;
}

void Simulation::SendSnapshot()
{
	PROFILE_FUNCTION();
	// remove any client disconnected
	this->ScanForDisconnects();

	// all new Entities
	this->SendEntities(m_addedEntities);
	m_addedEntities.clear();

	// all destroyed Entities
	this->SendRemoveEntities(m_removedEntities);
	m_removedEntities.clear();

	if (m_pCurrentScene == m_pGameScene)
	{
		// Positions
		network::message<GameMsg> msg;
		msg.header.id = GameMsg::Game_Snapshot;

		
		uint32_t count = 0;
		for (const auto& e : m_updatedEntities)
		{
			if (!e.IsNull())
			{
				std::bitset<ecs::Component::COMPONENT_MAX> compSet;
				for (int i = 0; i < ecs::Component::COMPONENT_COUNT; i++)
				{
					switch (i)
					{
					case ecs::Component::TRANSFORM:
					{
						comp::Transform* t = e.GetComponent<comp::Transform>();
						if (t)
						{
							compSet.set(ecs::Component::TRANSFORM);
							msg << *t;
						}
						break;
					}
					case ecs::Component::HEALTH:
					{
						comp::Health* h = e.GetComponent<comp::Health>();
						if (h)
						{
							compSet.set(ecs::Component::HEALTH);
							msg << *h;
						}
					}
					default:
						break;
					}
				}
				msg << static_cast<uint32_t>(compSet.to_ulong());
				msg << e.GetComponent<comp::Network>()->id;
				count++;
			}
		}
		msg << count;
		m_updatedEntities.clear();
		/*
		m_pCurrentScene->ForEachComponent<comp::Network, comp::Transform>([&]
		(comp::Network& n, comp::Transform& t)
			{
				msg << t << n.id;
				i++;
			});
		msg << i;
		*/
		/*
		i = 0;
		m_pCurrentScene->ForEachComponent<comp::Network, comp::Health>([&](comp::Network& n, comp::Health h)
			{
				msg << h << n.id;
				i++;
			});
		msg << i;
		*/

#if DEBUG_SNAPSHOT
		i = 0;
		m_pCurrentScene->ForEachComponent<comp::Network, comp::BoundingOrientedBox>([&]
		(comp::Network& n, comp::BoundingOrientedBox& b)
			{
				msg << b << n.id;
				i++;
			});
		msg << i;
#endif

		msg << this->GetTick();

		this->Broadcast(msg);
	}
	else if (m_pCurrentScene == m_pLobbyScene)
	{
		this->UpdateLobby();
	}
}

void Simulation::Update(float dt)
{
	PROFILE_FUNCTION();
	if (m_pCurrentScene)
		m_pCurrentScene->Update(dt);
}

void Simulation::UpdateInput(InputState state, uint32_t playerID)
{
	if (m_pCurrentScene != m_pGameScene)
		return;

	if (m_players.find(playerID) != m_players.end())
	{
		m_playerInputs[m_players.at(playerID)] = state;
	}
}

void Simulation::NextTick()
{
	this->m_tick++;
}

uint32_t Simulation::GetTick() const
{
	return this->m_tick;
}

void Simulation::Broadcast(network::message<GameMsg>& msg, uint32_t exclude)const
{
	auto it = m_players.begin();

	while (it != m_players.end())
	{
		if (exclude != it->first)
		{
			m_pServer->SendToClient(it->first, msg);
		}
		it++;
	}
}

void Simulation::ScanForDisconnects()
{
	auto it = m_players.begin();

	while (it != m_players.end())
	{
		if (!m_pServer->isClientConnected(it->first))
		{
			it = this->RemovePlayer(it);
		}
		else
		{
			it++;
		}
	}
}

void Simulation::OnNetworkEntityCreate(entt::registry& reg, entt::entity entity)
{
	Entity e(reg, entity);
	// Network has surely been added
	comp::Network* net = e.GetComponent<comp::Network>();
	if (net->id == UINT32_MAX)
	{
		net->id = GetUniqueID();
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
	m_updatedEntities.push_back(e);
}

HeadlessScene* Simulation::GetLobbyScene() const
{
	return m_pLobbyScene;
}

HeadlessScene* Simulation::GetGameScene() const
{
	return m_pGameScene;
}

void Simulation::SendEntity(Entity e, uint32_t exclude)const
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_AddEntity;

	InsertEntityIntoMessage(e, msg);

	msg << 1U;

	this->Broadcast(msg, exclude);
}

void Simulation::SendEntities(const std::vector<Entity>& entities) const
{
	if (entities.size() == 0)
		return;

	int32_t count = min(entities.size(), 10);
	size_t sent = 0;
	do
	{
		message<GameMsg> msg;
		msg.header.id = GameMsg::Game_AddEntity;
		for (size_t i = sent; i < sent + count; i++)
		{
			if (entities[i].IsNull())
			{
				count--;
				continue;
			}
			this->InsertEntityIntoMessage(entities[i], msg);
		}

		msg << static_cast<uint32_t>(count);

		this->Broadcast(msg);
		sent += count;
		count = min(entities.size() - sent, 10);
	} while (sent < entities.size());
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
		});
	msg << count;

	this->m_pServer->SendToClient(playerID, msg);
}

void Simulation::SendRemoveAllEntitiesToPlayer(uint32_t playerID) const
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_RemoveEntity;

	uint32_t count = 0;
	m_pGameScene->ForEachComponent<comp::Network>([&](comp::Network& n)
		{
			msg << n.id;
			count++;
		});
	msg << count;

	m_pServer->SendToClient(playerID, msg);
}

void Simulation::SendRemoveSingleEntity(Entity e) const
{
	comp::Network* net = e.GetComponent<comp::Network>();
	if (net)
	{
		this->SendRemoveSingleEntity(net->id);
	}
	else
	{
		LOG_WARNING("Tried to remove entity without network component");
	}
}

void Simulation::SendRemoveSingleEntity(uint32_t networkID) const
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_RemoveEntity;

	msg << (uint32_t)networkID << 1U;

	this->Broadcast(msg);
}

void Simulation::SendRemoveEntities(message<GameMsg>& msg)const
{
	this->Broadcast(msg);
}

void Simulation::SendRemoveEntities(const std::vector<uint32_t> entitiesNetIDs) const
{
	if (entitiesNetIDs.size() == 0)
		return;

	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_RemoveEntity;

	for (uint32_t e : entitiesNetIDs)
	{
		msg << e;
	}
	msg << static_cast<uint32_t>(entitiesNetIDs.size());

	this->Broadcast(msg);
}

uint32_t Simulation::GetUniqueID()
{
	return m_pServer->PopNextUniqueID();
}
