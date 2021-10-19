#include "NetServerPCH.h"
#include "Simulation.h"

Simulation::Simulation(Server* pServer, HeadlessEngine* pEngine)
	: m_pServer(pServer)
	  , m_pEngine(pEngine), m_pLobbyScene(nullptr), m_pGameScene(nullptr), m_pCurrentScene(nullptr)
{
	this->m_gameID = 0;
}

bool Simulation::JoinLobby(uint32_t playerID, uint32_t gameID)
{
	// Send to client the message with the new game ID
	message<GameMsg> msg;
	msg.header.id = GameMsg::Lobby_Accepted;
	msg << gameID;

	m_pServer->SendToClient(m_pServer->GetConnection(playerID), msg);

	// Add the players to the simulation on that specific client
	this->AddPlayer(playerID);

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
		});

	m_pCurrentScene = m_pGameScene; // todo temp

	// Add player that created Lobby
	AddPlayer(playerID);

	// Send back gameID so Client knows what game it is in
	message<GameMsg> msg;
	msg.header.id = GameMsg::Lobby_Accepted;
	msg << m_gameID;
	m_pServer->SendToClient(m_connections[playerID], msg);

	return true;
}

// TODO ADD PLAYER FUNCTIONALITY
bool Simulation::AddPlayer(uint32_t playerID)
{
	LOG_INFO("Player with ID: %ld added to the game!", playerID);
	m_connections[playerID] = m_pServer->GetConnection(playerID);


	
	// Create Player entity in Game scene
	Entity player = m_pGameScene->CreateEntity();
	player.AddComponent<comp::Transform>();
	player.AddComponent<comp::Velocity>();
	player.AddComponent<comp::Network>()->id = playerID;
	player.AddComponent<comp::Player>()->runSpeed = 10.f;
	player.AddComponent<comp::BoundingOrientedBox>();
	
	// Create Entity to symbolize player in Lobby scene
	player = m_pLobbyScene->CreateEntity();
	player.AddComponent<comp::Network>()->id = playerID;

	return true;
}

bool Simulation::RemovePlayer(uint32_t playerID)
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_RemovePlayer;
	msg << playerID;

	this->Broadcast(msg);

	m_pLobbyScene->ForEachComponent<comp::Network>([playerID](Entity e, comp::Network& n) 
		{
			if (n.id == playerID)
			{
				e.Destroy();
			}
		});

	m_pGameScene->ForEachComponent<comp::Network>([playerID](Entity e, comp::Network& n)
		{
			if (n.id == playerID)
			{
				e.Destroy();
			}
		});


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
