#include "NetServerPCH.h"
#include "Simulation.h"

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

Simulation::Simulation(Server* pServer, HeadlessEngine* pEngine)
	: m_pServer(pServer)
	, m_pEngine(pEngine), m_pLobbyScene(nullptr), m_pGameScene(nullptr), m_pCurrentScene(nullptr)
{
	this->m_gameID = 0;
	this->m_tick = 0;
}

bool Simulation::JoinLobby(uint32_t playerID, uint32_t gameID)
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
		this->AddPlayer(playerID);

		message<GameMsg> msg2;
		msg2.header.id = GameMsg::Lobby_PlayerJoin;
		msg << playerID;

		this->Broadcast(msg2);
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

	this->SendRemoveAllEntitiesToPlayer(playerID);

	// Update the lobby for players.
	network::message<GameMsg> msg;
	msg.header.id = GameMsg::Lobby_PlayerLeft;
	msg << playerID;
	this->Broadcast(msg);

	return true;
}


bool Simulation::Create(uint32_t playerID, uint32_t gameID)
{
	this->m_gameID = gameID;
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
					if (stats && stats->cooldownTimer <= 0.0f)
					{
						stats->isAttacking = true;
						stats->targetRay = input.mouseRay;
					}
				}
			}

			//  run all game logic systems
			Systems::CharacterMovement(scene, e.dt);
			Systems::MovementSystem(scene, e.dt);
			Systems::MovementColliderSystem(scene, e.dt);
			Systems::CheckCollisions<comp::BoundingOrientedBox, comp::BoundingOrientedBox>(scene, e.dt);
			Systems::CheckCollisions<comp::BoundingOrientedBox, comp::BoundingSphere>(scene, e.dt);
			Systems::CombatSystem(scene, e.dt);

		});

	//On collision event add entities as pair in the collision system
	m_pGameScene->on<ESceneCollision>([&](const ESceneCollision& e, HeadlessScene& scene)
		{
			CollisionSystem::Get().AddPair(e.obj1, e.obj2);
			CollisionSystem::Get().OnCollision(e.obj1, e.obj2);
		});

	m_pGameScene->GetRegistry()->on_construct<comp::Network>().connect<&Simulation::OnNetworkEntityCreate>(this);
	m_pGameScene->GetRegistry()->on_destroy<comp::Network>().connect<&Simulation::OnNetworkEntityDestroy>(this);


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
	e2.AddComponent<comp::Transform>()->position = { -250, -2, 300 };
	e2.AddComponent<comp::MeshName>()->name = "GameScene.obj";
	e2.AddComponent<comp::Tag<TagType::STATIC>>();
	// send entity
	e2.AddComponent<comp::Network>();

	// --- END OF THE WORLD ---

	m_pCurrentScene = m_pLobbyScene;

	// Automatically join created lobby
	JoinLobby(playerID, gameID);

	return true;
}

void Simulation::Destroy()
{
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

#ifdef _DEBUG
			// Debugging allow only one player to start.
			m_pCurrentScene = m_pGameScene;
			// Start the game.
			network::message<GameMsg> msg;
			msg.header.id = GameMsg::Game_Start;
			this->Broadcast(msg);
			return;
#endif
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

bool Simulation::AddPlayer(uint32_t playerID)
{
	if (!m_pServer->isClientConnected(playerID))
	{
		return false;
	}

	// Send all entities in Game Scene to new player
	this->SendAllEntitiesToPlayer(playerID);

	// Create Player entity in Game scene
	Entity player = m_pGameScene->CreateEntity();
	player.AddComponent<comp::Transform>();
	player.AddComponent<comp::Velocity>();
	player.AddComponent<comp::MeshName>()->name = "Arrow.fbx";
	player.AddComponent<comp::Player>()->runSpeed = 10.f;
	*player.AddComponent<comp::CombatStats>() = { 1.0f, 20.f, 1.0f, false, false };
	player.AddComponent<comp::Health>();
	player.AddComponent<comp::BoundingOrientedBox>();

	//Collision will handle this entity as a dynamic one
	player.AddComponent<comp::Tag<TagType::DYNAMIC>>();

	m_players[playerID] = player;
	// Network component has to be added last for entire entity to be sent
	player.AddComponent<comp::Network>(playerID); // send new Player entity to all clients

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

	// Network component has to be added last for entire entity to be sent
	enemy.AddComponent<comp::Network>();

	return true;
}

bool Simulation::RemovePlayer(uint32_t playerID)
{
	Entity player = m_players.at(playerID);
	if (m_playerInputs.find(player) != m_playerInputs.end())
	{
		m_playerInputs.erase(player);
	}
	m_players.erase(playerID);

	if (!player.Destroy())
	{
		return false;
	}
	LOG_INFO("Removed player %u from scene", player.GetComponent<comp::Network>()->id);

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

	if (m_pCurrentScene == m_pGameScene)
	{
		// remove any client disconnected
		this->ScanForDisconnects();
		
		// all new Entities
		this->SendEntities(m_addedEntities);
		m_addedEntities.clear();

		// all destroyed Entities
		this->SendRemoveEntities(m_removedEntities);
		m_removedEntities.clear();

		// Positions
		network::message<GameMsg> msg;
		msg.header.id = GameMsg::Game_Snapshot;

		uint32_t i = 0;
		m_pCurrentScene->ForEachComponent<comp::Network, comp::Transform>([&](comp::Network& n, comp::Transform& t)
			{
				msg << t << n.id;
				i++;
			});
		msg << i;

		msg << this->GetTick();

		this->Broadcast(msg);
	}

}

void Simulation::Update(float dt)
{
	if (m_pCurrentScene)
		m_pCurrentScene->Update(dt);
}

void Simulation::UpdateInput(InputState state, uint32_t playerID)
{
	if (m_pCurrentScene != m_pGameScene)
		return;

	if (m_players.find(playerID) == m_players.end())
	{
		LOG_ERROR("Invalid Player ID when updating input: %u", playerID);
		return;
	}
	
	m_playerInputs[m_players.at(playerID)] = state;
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
	if (net->id == -1)
	{
		net->id = m_pServer->PopNextUniqueID();
	}
	m_addedEntities.push_back(e);
}

void Simulation::OnNetworkEntityDestroy(entt::registry& reg, entt::entity entity)
{
	Entity e(reg, entity);
	// Network has not been destroyed yet
	m_removedEntities.push_back(e.GetComponent<comp::Network>()->id);
}

HeadlessScene* Simulation::GetLobbyScene() const
{
	return m_pLobbyScene;
}

HeadlessScene* Simulation::GetGameScene() const
{
	return m_pGameScene;
}

void Simulation::SendEntity(Entity e)const
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_AddEntity;

	InsertEntityIntoMessage(e, msg);

	msg << 1U;

	this->Broadcast(msg);
}

void Simulation::SendEntities(const std::vector<Entity>& entities) const
{
	if (entities.size() == 0)
		return;

	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_AddEntity;
	for (const auto& e : entities)
	{
		this->InsertEntityIntoMessage(e, msg);
	}

	msg << static_cast<uint32_t>(entities.size());

	this->Broadcast(msg);
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
