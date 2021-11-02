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

message<GameMsg> Simulation::SingleEntityMessage(Entity entity)const
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
	this->AICreateNodes();
}

bool Simulation::JoinLobby(uint32_t playerID, uint32_t gameID)
{
	// Send to client the message with the new game ID
	if (m_players.size() < MAX_PLAYER_PER_LOBBY)
	{
		message<GameMsg> msg;
		msg.header.id = GameMsg::Lobby_Accepted;
		msg << gameID;

		m_pServer->SendToClient(playerID, msg);

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

		// Any late joining players that join a lobby that is already inside game will be directed onto game scene.
		if (m_pCurrentScene == m_pGameScene)
		{
			network::message<GameMsg> msg3;
			msg.header.id = GameMsg::Game_Start;
			m_pServer->SendToClient(playerID, msg3);
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
		m_pServer->SendToClient(playerID, msg);
	}

	return true;
}

bool Simulation::LeaveLobby(uint32_t playerID, uint32_t gameID)
{
	if (!this->RemovePlayer(playerID))
	{
		return false;
	}

	m_players.erase(playerID);

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

	uint32_t count = 0;

	m_pGameScene->ForEachComponent<comp::Network>([&](comp::Network& n)
	{
		msg << n.id;
		count++;
	});
	msg << count;

	m_pServer->SendToClient(playerID, msg);

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
			e.GetComponent<comp::Velocity>()->vel = sm::Vector3((float)input.axisHorizontal, 0.f, (float)input.axisVertical) * e.GetComponent<comp::Player>()->runSpeed;

			// check if attacking
			if (input.leftMouse)
			{
				comp::CombatStats* stats = e.GetComponent<comp::CombatStats>();
				if (stats->cooldownTimer <= 0.0f)
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
		Systems::AISystem(scene);

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
	e.AddComponent<comp::Transform>()->position = sm::Vector3(0, 0, 0);
	e.AddComponent<comp::MeshName>()->name = "Chest.obj";
	e.AddComponent<comp::BoundingOrientedBox>()->Extents = sm::Vector3(2.f, 2.f, 2.f);
	e.AddComponent<comp::NPC>();
	e.AddComponent<comp::Health>();
	*e.AddComponent<comp::CombatStats>() = { 1.0f, 20.f, 1.0f, false, false };
	e.AddComponent<comp::Tag<TagType::STATIC>>();
	// ---END OF DEBUG---

	// --- WORLD ---
	Entity e2 = m_pGameScene->CreateEntity();
	e2.AddComponent<comp::Network>()->id = m_pServer->PopNextUniqueID();
	e2.AddComponent<comp::Transform>();
	e2.AddComponent<comp::MeshName>()->name = "GameScene.obj";
	e2.AddComponent<comp::Tag<TagType::STATIC>>();
	// --- END OF THE WORLD ---

	m_pCurrentScene = m_pLobbyScene;


	//Gridsystem
	GridProperties_t gridOption;
	m_grid.Initialize(gridOption.mapSize, gridOption.position, gridOption.fileName, m_pGameScene);

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
	return m_players.empty();
}

bool Simulation::AICreateNodes()
{
	//m_pGameScene->ForEachComponent<comp::Tile>([=](Entity entity, comp::Tile& tile) 
	//{
	//	//LOG_INFO("Tile ID: %lf  %lf",tile.gridID.x, tile.gridID.y);
	//});
	std::vector<Entity>* tiles = m_grid.GetTiles();
	for (Entity& tile : *tiles)
	{
		comp::Tile* tileComponent = tile.GetComponent<comp::Tile>();
		LOG_INFO("Tile ID: %lf  %lf", tileComponent->gridID.x, tileComponent->gridID.y);

	}
	//std::vector<std::string> file = OpenFile("Other/NodeInfo.txt");//replace with actual name
	//int currentIndex = 0;
	//if (file.size() == 0)
	//{
	//	LOG_ERROR("Error Opening NodeFile!");
	//}
	////Creating nodes
	//LOG_INFO("Creating nodes");
	//while (file[currentIndex] != "")
	//{
	//	std::stringstream ss(file.at(currentIndex));
	//	int ID = 0;
	//	float posX = 0, posZ = 0;
	//	ss >> ID >> posX >> posZ;
	//	Entity node = m_pGameScene->CreateEntity();
	//	node.AddComponent<comp::Node>();

	//	currentIndex++;
	//}
	//LOG_INFO("Connecting Nodes");
	//for (int i = currentIndex + 1; i < file.size(); i++)
	//{
	//	std::stringstream ss(file.at(i));
	//	int ID1 = 0, ID2 = 0;
	//	ss >> ID1 >> ID2;
	//	this->ConnectNodes(this->GetAINodeById(ID1), this->GetAINodeById(ID2));
	//}

	return true;
}

bool Simulation::AIAStarSearch()
{
	//TEMP, needs to get from Entity
	comp::Node* currentNode;

	std::vector<comp::Node*> closedList, openList;
	//AIHANDLER->path.clear();
	comp::Node* startingNode = currentNode, * goalNode = currentNode;
	openList.push_back(startingNode);
	startingNode->f = 0.f;
	startingNode->g = 0.f;
	startingNode->h = 0.f;
	startingNode->parent = startingNode;

	while (goalNode == currentNode)
	{
		//goalNode = New target node;
	}

	comp::Node* nodeToAdd = nullptr;
	int index = 0;
	while (!openList.empty() && nodeToAdd != goalNode)
	{
		nodeToAdd = openList.at(0);
		int indexToPop = 0;
		bool stop = false;
		for (unsigned int i = 0; i < openList.size(); i++)
		{
			if (openList.at(i)->f < nodeToAdd->f)
			{
				nodeToAdd = openList.at(i);
				indexToPop = i;
			}
		}
		openList.erase(openList.begin() + indexToPop);

		//Neighbors

		std::vector<comp::Node*> neighbors = nodeToAdd->connections;

		for (comp::Node* neighbor : neighbors)
		{
			if (neighbor->parent != nodeToAdd && neighbor != nodeToAdd)
			{
				if (!neighbor->parent)
				{
					neighbor->parent = nodeToAdd;
				}
				if (neighbor == goalNode)
				{
					nodeToAdd = goalNode;
					break;
				}
				if (neighbor->f == FLT_MAX)
				{
					float tempF = 0, tempG = 0, tempH = 0;

					tempG = nodeToAdd->g + (nodeToAdd->position - neighbor->position).Length();
					tempH = (goalNode->position - nodeToAdd->position).Length(); //Using euclidean distance
					tempF = tempG + tempH;
					neighbor->f = tempF;
					neighbor->g = tempG;
					neighbor->h = tempH;
				}
				stop = false;
				for (unsigned int i = 0; i < openList.size() && !stop; i++)
				{
					if (openList.at(i)->id == neighbor->id)
					{
						stop = true;
					}
				}
				if (closedList.size() > 0)
				{
					for (unsigned int i = 0; i < closedList.size() && !stop; i++)
					{
						if (closedList.at(i)->id == neighbor->id)
						{
							stop = true;
						}
					}
					if (!stop)
					{
						openList.push_back(neighbor);
					}
				}
				else
				{
					openList.push_back(neighbor);
				}
			}
		}

		closedList.push_back(nodeToAdd);

		index++;

	}

	//TracePath

	while (goalNode != startingNode)
	{
		//Insert currentNode to the path
		currentNode = currentNode->parent;
	}

	/*
	for (unsigned int i = 0; i < AIHANDLER->allNodes.size(); i++)
	{
		AIHANDLER->allNodes.at(i)->ResetFGH();
		AIHANDLER->allNodes.at(i)->ResetParent();
	}
	AIHANDLER->nrOfAstar++;*/
	return true;
}

bool Simulation::AddPlayer(uint32_t playerID)
{
	if (!m_pServer->isClientConnected(playerID))
	{
		return false;
	}

	// Send all entities in Game Scene to new player
	m_pServer->SendToClient(playerID, AllEntitiesMessage());

	// Create Player entity in Game scene
	Entity player = m_pGameScene->CreateEntity();
	player.AddComponent<comp::Transform>();
	player.AddComponent<comp::Velocity>();
	player.AddComponent<comp::MeshName>()->name = "Arrow.fbx";
	player.AddComponent<comp::Network>()->id = playerID;
	player.AddComponent<comp::Player>()->runSpeed = 10.f;
	*player.AddComponent<comp::CombatStats>() = { 1.0f, 20.f, 1.0f, false, false };
	player.AddComponent<comp::Health>();
	player.AddComponent<comp::BoundingOrientedBox>();

	//Collision will handle this entity as a dynamic one
	player.AddComponent<comp::Tag<TagType::DYNAMIC>>();

	// Setup players temp...
	for (int i = 0; i < 2; i++)
	{
		if (m_playerDecisions[i].playerID == -1)
		{
			m_playerDecisions[i].playerID = playerID;
			break;
		}
	}
	// send new Player to all other clients
	m_players[playerID] = player;
	this->Broadcast(SingleEntityMessage(player));

	return true;
}

bool Simulation::AddEnemy()
{
	// Create Enemy entity in Game scene.
	Entity enemy = m_pGameScene->CreateEntity();
	enemy.AddComponent<comp::Transform>();
	enemy.AddComponent<comp::Network>()->id = m_pServer->PopNextUniqueID();
	const unsigned char BAD = 8;
	enemy.AddComponent<comp::Tag<BAD>>();
	enemy.AddComponent<comp::Health>();

	return true;
}

bool Simulation::RemovePlayer(uint32_t playerID)
{
	Entity player = m_players.at(playerID);
	if (m_playerInputs.find(player) != m_playerInputs.end())
	{
		m_playerInputs.erase(player);
	}
	if (!player.Destroy())
	{
		return false;
	}

	LOG_INFO("Removed player %u from scene", playerID);

	return true;
}
bool Simulation::AddNPC(uint32_t npcId)
{
	//LOG_INFO("NPC with ID: %ld added to game!", npcId);

	Entity npc = m_pGameScene->CreateEntity();
	npc.AddComponent<comp::Transform>()->position = sm::Vector3(0.f, 0.f, 0.f);
	npc.AddComponent<comp::Velocity>();
	npc.AddComponent<comp::MeshName>()->name = "StreetLamp.obj";
	npc.AddComponent<comp::NPC>();
	npc.AddComponent<comp::Network>()->id = npcId;
	npc.AddComponent<comp::BoundingOrientedBox>();

	CollisionSystem::Get().AddOnCollision(npc, [&](Entity other)
	{
		comp::NPC* otherNPC = m_pCurrentScene->GetRegistry()->try_get<comp::NPC>(other);
		if (otherNPC)
		{
			LOG_INFO("NPC COLLISION!");
		}
	});
	Broadcast(SingleEntityMessage(npc));
	return true;
}
bool Simulation::RemoveNPC(uint32_t npcId)
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_RemoveEntity;
	msg << npcId << 1U;
	this->Broadcast(msg);
	m_pGameScene->ForEachComponent<comp::Network>([npcId](Entity e, comp::Network& n)
	{
		if (n.id == npcId)
		{
			LOG_INFO("Removed NPC %u from game scene", n.id);
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

	msg << this->GetTick();
	this->ScanForDisconnects();
	this->Broadcast(msg);
}

void Simulation::Update(float dt)
{
	if (m_pCurrentScene)
		m_pCurrentScene->Update(dt);
}

void Simulation::UpdateInput(InputState state, uint32_t playerID)
{
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

void Simulation::Broadcast(network::message<GameMsg>& msg, uint32_t exclude)
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
			this->RemovePlayer(it->first);

			message<GameMsg> msg;
			msg.header.id = GameMsg::Game_RemoveEntity;
			msg << it->first << 1U;

			it = m_players.erase(it);
			this->Broadcast(msg);
		}
		else
		{
			it++;
		}
	}
}

std::vector<std::string> Simulation::OpenFile(std::string filePath)
{
	std::ifstream nodeFile;
	nodeFile.open(filePath, std::ios::app);
	std::string currentLine;
	std::vector<std::string> allLines;
	if (nodeFile.is_open())
	{
		while (!nodeFile.eof())
		{
			std::getline(nodeFile, currentLine);
			allLines.push_back(currentLine);
		}
	}
	nodeFile.close();

	return allLines;
}

void Simulation::ConnectNodes(comp::Node* node1, comp::Node* node2)
{
	if (node1 && node2)
	{
		node1->connections.push_back(node2);
		node2->connections.push_back(node1);
	}
}

comp::Node* Simulation::GetAINodeById(uint32_t id)
{
	m_pGameScene->ForEachComponent<comp::Node>([&](comp::Node& n)
	{
		if (n.id == id)
		{
			return n;
		}
	});
	return nullptr;
}

HeadlessScene* Simulation::GetLobbyScene() const
{
	return m_pLobbyScene;
}

HeadlessScene* Simulation::GetGameScene() const
{
	return m_pGameScene;
}
