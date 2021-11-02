#include "NetServerPCH.h"
#include "ServerGame.h"

using namespace std::placeholders;

ServerGame::ServerGame()
	:m_server(std::bind(&ServerGame::CheckIncoming, this, _1))
{
	m_nGameID = 0;
}

ServerGame::~ServerGame()
{

}

void ServerGame::InputThread()
{
	std::string input;
	while (1) {
		std::cin >> input;
		if (input == "/stop")
		{
			m_server.Stop();
			this->Shutdown();
			break;
		}
		else if (input == "/info")
		{
			LOG_INFO("INFO:")
				for (const auto& sim : m_simulations)
				{
					LOG_INFO("-------Simulation %u-------", sim.first);
					LOG_INFO("LOBBY SCENE:");
					LOG_INFO("\tEntity Count: %u", (unsigned int)sim.second->GetLobbyScene()->GetRegistry()->size());
					sim.second->GetLobbyScene()->ForEachComponent<comp::Network>([](Entity e, comp::Network& n)
						{
							LOG_INFO("\tEntity: %d", (entt::entity)e);
							LOG_INFO("\tNetwork id: %u", n.id);
						});

					LOG_INFO("GAME SCENE:");
					LOG_INFO("\tEntity Count: %u\n", (unsigned int)sim.second->GetGameScene()->GetRegistry()->size());
					sim.second->GetGameScene()->ForEachComponent<comp::Network>([](Entity e, comp::Network& n)
						{
							LOG_INFO("\tEntity: %d", (entt::entity)e);
							LOG_INFO("\tNetwork id: %u", n.id);
						});

				}
		}
	}
}

bool ServerGame::OnStartup()
{
	if (!m_server.Start(4950))
	{
		LOG_ERROR("Failed to start server");
		exit(0);
	}

	m_inputThread = std::thread(&ServerGame::InputThread, this);

	LoadMapColliders("SceneBoundingBoxes.obj");
	//LoadMapColliders("MapBounds.obj")

	return true;
}

void ServerGame::OnShutdown()
{
	m_inputThread.join();
}


void ServerGame::UpdateNetwork(float deltaTime)
{
	static float timer = 0.0f;
	timer += deltaTime;
	if (timer >= 1.0f)
	{
		LOG_INFO("Update: %f", 1.f / deltaTime);
		timer = 0.0f;
	}

	// Check incoming messages
	this->m_server.Update();

	// Update the simulations
	for (auto it = m_simulations.begin(); it != m_simulations.end();)
	{
		if (it->second->IsEmpty())
		{
			it->second->Destroy();
			LOG_INFO("Destroyed empty lobby %d", it->first);
			it = m_simulations.erase(it);
		}
		else
		{
			// Update the simulation
			it->second->Update(deltaTime);
			// Send the snapshot of the updated simulation to all clients in the sim
			it->second->SendSnapshot();
			it->second->NextTick();
			it++;
		}
	}
}

bool ServerGame::LoadMapColliders(const std::string& filename)
{
	std::string filepath = BOUNDSPATH + filename;
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile
	(
		filepath, 
		aiProcess_Triangulate			| 
		aiProcess_JoinIdenticalVertices |
		aiProcess_FlipWindingOrder		|
		aiProcess_MakeLeftHanded
	);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
#ifdef _DEBUG
		LOG_WARNING("[Bounds] Assimp error: %s", importer.GetErrorString());
#endif 
		importer.FreeScene();
		return false;
	}

	if (!scene->HasMeshes())
	{
#ifdef _DEBUG
		LOG_WARNING("[Bounds] has no meshes...");
#endif 
		importer.FreeScene();
		return false;
	}

	// Go through all the meshes and create boundingboxes for them
	for (UINT i = 0; i < scene->mNumMeshes; i++)
	{
		const aiMesh* mesh = scene->mMeshes[i];
		size_t count = mesh->mNumVertices;
		std::vector<dx::XMFLOAT3> vertices;
		vertices.reserve(count);

		//Go through all the vertices
		for (UINT v = 0; v < count; v++)
		{
			aiVector3D aivec = mesh->mVertices[v];
			vertices.push_back({ aivec.x, aivec.y, aivec.z });
		}

		//Create a bob from all the points and the orientation will be calculated and add to vector
		dx::BoundingOrientedBox bob;
		dx::BoundingOrientedBox::CreateFromPoints(bob, count, &vertices[0], sizeof(dx::XMFLOAT3));
		m_mapColliders.push_back(bob);
	}

	return true;
}

void ServerGame::CheckIncoming(message<GameMsg>& msg)
{
	switch (msg.header.id)
	{
	case GameMsg::Server_GetPing:
	{
		uint32_t playerID;
		msg >> playerID;
		this->m_server.SendToClient(playerID, msg);
		LOG_INFO("Client on with ID: %ld is pinging server", playerID);
		break;
	}
	case GameMsg::Lobby_Create:
	{
		uint32_t playerID;
		msg >> playerID;
		std::string namePlate;
		msg >> namePlate;
		if (this->CreateSimulation(playerID, namePlate))
		{
			LOG_INFO("Created Game lobby!");
		}
		else
		{
			LOG_ERROR("Failed to create Lobby!");
		}
		break;
	}
	case GameMsg::Lobby_Join:
	{
		uint32_t gameID;
		msg >> gameID;
		uint32_t playerID;
		msg >> playerID;
		std::string namePlate;
		msg >> namePlate;
		if (m_simulations.find(gameID) != m_simulations.end())
		{
			m_simulations[gameID]->JoinLobby(playerID, gameID, namePlate);
		}
		else
		{
			message<GameMsg> invalidLobbyMsg;
			invalidLobbyMsg.header.id = GameMsg::Lobby_Invalid;
			invalidLobbyMsg << std::string("Request denied: Invalid Lobby");
			LOG_WARNING("Request denied: Invalid Lobby");
			m_server.SendToClient(playerID, invalidLobbyMsg);
		}
		break;
	}
	case GameMsg::Lobby_Leave:
	{
		uint32_t gameID;
		msg >> gameID;
		uint32_t playerID;
		msg >> playerID;
		if (m_simulations.find(gameID) != m_simulations.end())
		{
			if (m_simulations[gameID]->LeaveLobby(playerID, gameID)) {

				// Send to client the message with the new game ID
				message<GameMsg> accMsg;
				accMsg.header.id = GameMsg::Lobby_AcceptedLeave;

				m_server.SendToClient(playerID, accMsg);
				break;
			}
		}

		message<GameMsg> invalidLobbyMsg;
		invalidLobbyMsg.header.id = GameMsg::Lobby_Invalid;
		invalidLobbyMsg << std::string("Player could not leave Lobby");
		LOG_WARNING("Request denied: Player could not leave Lobby");
		m_server.SendToClient(playerID, invalidLobbyMsg);

		break;
	}
	case GameMsg::Game_PlayerInput:
	{
		InputState input;
		uint32_t playerID;
		uint32_t gameID;

		msg >> input >> gameID >> playerID;

		if (m_simulations.find(gameID) != m_simulations.end())
		{
			m_simulations.at(gameID)->UpdateInput(input, playerID);
		}
		else
		{
			LOG_WARNING("Invalid GameID for player input message");
		}

		break;
	}
	case GameMsg::Game_PlayerReady:
	{
		uint32_t playerID;
		uint32_t gameID;
		msg >> gameID >> playerID;

		if (m_simulations.find(gameID) != m_simulations.end())
		{
			m_simulations.at(gameID)->ReadyCheck(playerID);
		}

		break;
	}
	}
}

bool ServerGame::CreateSimulation(uint32_t playerID, const std::string& mainPlayerPlate)
{
	m_simulations[m_nGameID] = std::make_unique<Simulation>(&m_server, this);
	if (!m_simulations[m_nGameID]->Create(playerID, m_nGameID, &m_mapColliders, mainPlayerPlate))
	{
		m_simulations.erase(m_nGameID);
		return false;
	}

	m_nGameID++;

	return true;
}
