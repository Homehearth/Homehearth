#include "NetServerPCH.h"
#include "Simulation.h"

void Simulation::InsertEntityIntoMessage(Entity entity, message<GameMsg>& msg)
{
	std::bitset<ecs::Component::COMPONENT_MAX> compSet;

	for (uint32_t i = 0; i < ecs::Component::COMPONENT_COUNT; i++)
	{
		switch (i)
		{
		case ecs::Component::NETWORK:
		{
			compSet.set(ecs::Component::NETWORK);
			msg << entity.GetComponent<comp::Network>()->id;
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
				msg << b->Center << b->Extents << b->Orientation;
			}
			break;
		}
		case ecs::Component::BOUNDING_SPHERE:
		{
			comp::BoundingSphere* bs = entity.GetComponent<comp::BoundingSphere>();
			if (bs)
			{
				compSet.set(ecs::Component::BOUNDING_SPHERE);
				msg << bs->Center << bs->Radius;
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

message<GameMsg> Simulation::AllEntitiesMessage()
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

message<GameMsg> Simulation::SingleEntityMessage(Entity entity)
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_AddEntity;

	InsertEntityIntoMessage(entity, msg);

	msg << 1U;

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
	// Send to client the message with the new game ID
	if (m_players.size() < MAX_PLAYER_PER_LOBBY)
	{
		message<GameMsg> msg;
		msg.header.id = GameMsg::Lobby_Accepted;
		msg << gameID;

		m_pServer->SendToClient(m_pServer->GetConnection(playerID), msg);

		// Add the players to the simulation on that specific client
		this->AddPlayer(playerID);

		uint32_t player = -1;
		// Reset Lobby player.
		if (m_playerDecisions[0].playerID == playerID)
		{
			player = 1;
		}
		else if (m_playerDecisions[1].playerID == playerID)
		{
			player = 2;
		}

		// Update the lobby for players.
		network::message<GameMsg> msg2;
		msg2.header.id = GameMsg::Lobby_Update;
		const uint32_t nrOfPlayers = (const uint32_t)m_players.size();
		msg2 << nrOfPlayers << player << (uint8_t)1;
		this->Broadcast(msg2);
	}
	else
	{
		network::message<GameMsg> msg;
		msg.header.id = GameMsg::Lobby_Invalid;
		std::string fullLobby = "This lobby was full!";
		msg << fullLobby;
		m_pServer->SendToClient(m_pServer->GetConnection(playerID), msg);
	}

	return true;
}

bool Simulation::LeaveLobby(uint32_t playerID, uint32_t gameID)
{
	this->RemovePlayer(playerID);

	uint32_t player = -1;
	// Reset Lobby player.
	if (m_playerDecisions[0].playerID == playerID)
	{
		m_playerDecisions[0].playerID = -1;
		m_playerDecisions[0].isWantToStart = false;
		player = 1;
	}
	else if (m_playerDecisions[1].playerID == playerID)
	{
		m_playerDecisions[1].playerID = -1;
		m_playerDecisions[1].isWantToStart = false;
		player = 2;
	}

	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_RemoveEntity;

	// Remove from game.
	m_players.erase(m_players.find(playerID));

	
	uint32_t count = 0;
	m_pGameScene->ForEachComponent<comp::Network>([&](comp::Network& n)
		{
			msg << n.id;
			count++;
		});
	msg << count;

	m_pServer->SendToClient(m_pServer->GetConnection(playerID), msg);

	// Update the lobby for players.
	network::message<GameMsg> msg2;
	msg2.header.id = GameMsg::Lobby_Update;
	const uint32_t nrOfPlayers = (const uint32_t)m_players.size();
	msg2 << nrOfPlayers << player << (uint8_t)2;
	this->Broadcast(msg2);

	return true;
}


bool Simulation::Create(uint32_t playerID, uint32_t gameID)
{
	this->m_gameID = gameID;
	// Create Scenes associated with this Simulation
	m_pLobbyScene = &m_pEngine->GetScene("Lobby_" + std::to_string(gameID));
	m_pLobbyScene->on<ESceneUpdate>([=](const ESceneUpdate& e, HeadlessScene& scene) 
		{
			//LOG_INFO("LOBBY Scene %d", gameID);

		});

	m_pGameScene = &m_pEngine->GetScene("Game_" + std::to_string(gameID));
	m_pGameScene->on<ESceneUpdate>([=](const ESceneUpdate& e, HeadlessScene& scene)
		{
			Systems::MovementSystem(scene, e.dt);
			Systems::MovementColliderSystem(scene, e.dt);

			Systems::CheckCollisions<comp::BoundingOrientedBox, comp::BoundingOrientedBox>(scene);
			//LOG_INFO("GAME Scene %d", m_gameID);
		});

	//On collision event add entities as pair in the collision system
	m_pGameScene->on<ESceneCollision>([&](const ESceneCollision& e, HeadlessScene& scene)
		{
			CollisionSystem::Get().AddPair(e.obj1, e.obj2);
			CollisionSystem::Get().OnCollision(e.obj1, e.obj2);
		});

	// ---DEBUG ENTITY---
	Entity e = m_pGameScene->CreateEntity();
	e.AddComponent<comp::Network>()->id = m_pServer->PopNextUniqueID();
	e.AddComponent<comp::Transform>()->position = sm::Vector3(5, 2, 0);
	e.AddComponent<comp::MeshName>()->name = "Chest.obj";
	e.AddComponent<comp::Velocity>()->vel = sm::Vector3(0, -0.2f, 0);
	e.AddComponent<comp::BoundingSphere>();
	// ---END OF DEBUG---

	m_pCurrentScene = m_pLobbyScene; // todo Should be lobbyScene
	
	// Automatically join created lobby
	JoinLobby(playerID, gameID);

	return true;
}

void Simulation::Destroy()
{
	m_pGameScene->Clear();
	m_pLobbyScene->Clear();
}

void Simulation::UpdateLobby(const uint32_t& playerID)
{
	if (m_pCurrentScene == m_pLobbyScene)
	{
		// Update decisions.
		if (playerID == m_playerDecisions[0].playerID)
		{
			m_playerDecisions[0].isWantToStart = !m_playerDecisions[0].isWantToStart;
		}
		else if (playerID == m_playerDecisions[1].playerID)
		{
			m_playerDecisions[1].isWantToStart = !m_playerDecisions[1].isWantToStart;
		}

#ifdef _DEBUG
		// Debugging allow only one player to start.
		m_pCurrentScene = m_pGameScene;
		// Start the game.
		network::message<GameMsg> msg;
		msg.header.id = GameMsg::Game_Start;
		this->Broadcast(msg);
#endif

		// Start game when both wants to start game.
		if ((m_playerDecisions[1].isWantToStart & m_playerDecisions[0].isWantToStart) == true)
		{
			m_pCurrentScene = m_pGameScene;

			// Start the game.
			network::message<GameMsg> msg;
			msg.header.id = GameMsg::Game_Start;
			this->Broadcast(msg);
		}
	}
}

bool Simulation::IsEmpty() const
{
	return m_connections.empty();
}

// TODO ADD PLAYER FUNCTIONALITY
bool Simulation::AddPlayer(uint32_t playerID)
{

	LOG_INFO("Player with ID: %ld added to the game!", playerID);
	m_connections[playerID] = m_pServer->GetConnection(playerID);
	
	// Send all entities in Game Scene to new player
	m_pServer->SendToClient(m_pServer->GetConnection(playerID), AllEntitiesMessage());

	// Create Player entity in Game scene
	Entity player = m_pGameScene->CreateEntity();
	player.AddComponent<comp::Transform>();
	player.AddComponent<comp::Velocity>();
	player.AddComponent<comp::MeshName>()->name = "cube.obj";
	player.AddComponent<comp::Network>()->id = playerID;
	player.AddComponent<comp::Player>()->runSpeed = 10.f;
	player.AddComponent<comp::BoundingOrientedBox>();

	CollisionSystem::Get().AddOnCollision(player, [&](Entity player2)
		{
			comp::Player* otherPlayer = m_pCurrentScene->GetRegistry()->try_get<comp::Player>(player2);
			if(otherPlayer != nullptr)
			{
				//LOG_INFO("Collision!");
			}
		});

	// Setup players temp...
	for (int i = 0; i < 2; i++)
	{
		if (m_playerDecisions[i].playerID == -1)
		{
			m_playerDecisions[i].playerID = playerID;
			break;
		}
	}

	m_players.emplace(playerID, player);
	// send new Player to all other clients
	Broadcast(SingleEntityMessage(player));

	return true;
}

bool Simulation::RemovePlayer(uint32_t playerID)
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_RemoveEntity;
	msg << playerID << 1U;

	this->Broadcast(msg);

	m_pGameScene->ForEachComponent<comp::Network>([playerID](Entity e, comp::Network& n)
		{
			if (n.id == playerID)
			{
				LOG_INFO("Removed player %u from game scene", n.id);
				e.Destroy();
			}
		});

	m_connections.erase(playerID);

	return true;
}

void Simulation::SendSnapshot()
{
	network::message<GameMsg> msg;
	msg.header.id = GameMsg::Game_Snapshot;

	uint32_t i = 0;
	m_pCurrentScene->ForEachComponent<comp::Network, comp::Transform>([&](Entity e, comp::Network& n, comp::Transform& t) 
		{
			msg << t << n.id;
			i++;
		});
	msg << i;

	this->Broadcast(msg);
}

void Simulation::Update(float dt) 
{
	if(m_pCurrentScene)
		m_pCurrentScene->Update(dt);
}

void Simulation::Broadcast(network::message<GameMsg>& msg, uint32_t exclude)
{
	auto it = m_connections.begin();

	while (it != m_connections.end())
	{
		if (m_pServer->GetConnection(it->first) != INVALID_SOCKET)
		{
			if (exclude != it->first)
			{
				m_pServer->SendToClient(it->second, msg);
			}
			it++;
		}
		else
		{
			uint32_t playerID = it->first;
			it = m_connections.erase(it);
			this->RemovePlayer(playerID);
		}
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
