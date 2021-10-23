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
	message<GameMsg> msg;
	msg.header.id = GameMsg::Lobby_Accepted;
	msg << gameID;

	m_pServer->SendToClient(m_pServer->GetConnection(playerID), msg);

	// Add the players to the simulation on that specific client
	this->AddPlayer(playerID);

	return true;
}

bool Simulation::LeaveLobby(uint32_t playerID, uint32_t gameID)
{
	this->RemovePlayer(playerID);

	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_RemoveEntity;
	
	uint32_t count = 0;
	m_pGameScene->ForEachComponent<comp::Network>([&](comp::Network& n)
		{
			msg << n.id;
			count++;
		});
	msg << count;

	m_pServer->SendToClient(m_pServer->GetConnection(playerID), msg);

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

	m_pCurrentScene = m_pGameScene; // todo Should be lobbyScene
	
	// Automatically join created lobby
	JoinLobby(playerID, gameID);

	return true;
}

void Simulation::Destroy()
{
	m_pGameScene->Clear();
	m_pLobbyScene->Clear();
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

	CollisionSystem::Get().AddOnCollision(player, [=](entt::entity player2)
		{
			comp::Player* otherPlayer = m_pCurrentScene->GetRegistry()->try_get<comp::Player>(player2);
			if(otherPlayer != nullptr)
			{
				comp::BoundingOrientedBox* p2Obb = m_pCurrentScene->GetRegistry()->try_get<comp::BoundingOrientedBox>(player2);
				comp::BoundingOrientedBox* p1Obb = m_pCurrentScene->GetRegistry()->try_get<comp::BoundingOrientedBox>(player);
				comp::Transform* p2transform = m_pCurrentScene->GetRegistry()->try_get<comp::Transform>(player2);
				comp::Transform* p1transform = m_pCurrentScene->GetRegistry()->try_get<comp::Transform>(player);
				
				sm::Vector3 p2Corners[8];
				sm::Vector3 p1Corners[8];
				p2Obb->GetCorners(p2Corners);
				p1Obb->GetCorners(p1Corners);
				
				//Box1
				sm::Vector3 p1normalAxis[3];
				sm::Vector3 p2normalAxis[3];
				
				//Get the 3 AXIS from box1 needed to do the projection on
				p1normalAxis[0] = (p1Corners[1] - p1Corners[0]);
				p1normalAxis[0].Normalize();
				p1normalAxis[1] = (p1Corners[2] - p1Corners[1]);
				p1normalAxis[1].Normalize();
				p1normalAxis[2] = (p1Corners[0] - p1Corners[4]);
				p1normalAxis[2].Normalize();
				
				//Get the 3 AXIS from box2 needed to do the projection on
				p2normalAxis[0] = (p2Corners[1] - p2Corners[0]);
				p2normalAxis[0].Normalize();
				p2normalAxis[1] = (p2Corners[2] - p2Corners[1]);
				p2normalAxis[1].Normalize();
				p2normalAxis[2] = (p2Corners[0] - p2Corners[4]);
				p2normalAxis[2].Normalize();

				std::vector<sm::Vector3> p1Vectors;
				std::vector<sm::Vector3> p2Vectors;

				for(int i = 0; i < 9; i++)
				{
					if (i < 8)
					{
						p1Vectors.emplace_back(p1Corners[i]);
						p2Vectors.emplace_back(p2Corners[i]);
					}
				}
				
				//Get min-max for the axis for the box
				std::vector<MinMaxProj_t> minMaxProj;
				minMaxProj.push_back(CollisionSystem::Get().GetMinMax(p1Vectors, p1normalAxis[1]));
				minMaxProj.push_back(CollisionSystem::Get().GetMinMax(p2Vectors, p1normalAxis[1]));

				minMaxProj.push_back(CollisionSystem::Get().GetMinMax(p1Vectors, p1normalAxis[0]));
				minMaxProj.push_back(CollisionSystem::Get().GetMinMax(p2Vectors, p1normalAxis[0]));

				minMaxProj.push_back(CollisionSystem::Get().GetMinMax(p1Vectors, p1normalAxis[2]));
				minMaxProj.push_back(CollisionSystem::Get().GetMinMax(p2Vectors, p1normalAxis[2]));

				minMaxProj.push_back(CollisionSystem::Get().GetMinMax(p1Vectors, p2normalAxis[1]));
				minMaxProj.push_back(CollisionSystem::Get().GetMinMax(p2Vectors, p2normalAxis[1]));
												   
				minMaxProj.push_back(CollisionSystem::Get().GetMinMax(p1Vectors, p2normalAxis[0]));
				minMaxProj.push_back(CollisionSystem::Get().GetMinMax(p2Vectors, p2normalAxis[0]));
											   
				minMaxProj.push_back(CollisionSystem::Get().GetMinMax(p1Vectors, p2normalAxis[2]));
				minMaxProj.push_back(CollisionSystem::Get().GetMinMax(p2Vectors, p2normalAxis[2]));

				float depth = FLT_MAX;
				sm::Vector3 smallestVec(FLT_MAX, FLT_MAX, FLT_MAX);
				bool isValueSet = false;
				for (int i = 0; i < minMaxProj.size() - 1; i += 2)
				{
					sm::Vector3 gap;
					if (minMaxProj[i].maxProj < minMaxProj[i + 1].maxProj)
					{
						gap = p2Vectors[minMaxProj[i + 1].minInxed] - p1Vectors[minMaxProj[i].maxIndex];
					}	
					else if(minMaxProj[i].maxProj > minMaxProj[i + 1].maxProj)
					{
						gap = p1Vectors[minMaxProj[i].minInxed]- p2Vectors[minMaxProj[i + 1].maxIndex];
					}
					else
					{
						continue; // should not happend?
					}
					if(gap.Length() < depth)
					{
						depth = gap.Length();
						smallestVec = gap;
						isValueSet = true;
					}
				}

				//Reset to 0.001 (happens if boxes share exactly the same corners pos)
				if(!isValueSet)
				{
					smallestVec = sm::Vector3(0.01f, 0.01f, 0.01f);
				}
				
				float lengthVec = smallestVec.Dot(p1normalAxis[0]);
				int indexForLowestVec = 0;
				for(int i = 1; i < 3; i++)
				{
					if(abs(smallestVec.Dot(p1normalAxis[i])) > 0.000f && abs(smallestVec.Dot(p1normalAxis[i])) < abs(lengthVec) || lengthVec < 0.0001f && lengthVec > -0.00001f)
					{
						lengthVec = smallestVec.Dot(p1normalAxis[i]);
						indexForLowestVec = i;
					}
				}
				for (int i = 0; i < 3; i++)
				{
					if (abs(smallestVec.Dot(p2normalAxis[i])) > 0.000f && abs(smallestVec.Dot(p2normalAxis[i])) < abs(lengthVec) || lengthVec < 0.0001f && lengthVec > -0.00001f)
					{
						lengthVec = smallestVec.Dot(p2normalAxis[i]);
						indexForLowestVec = i;
					}
				}
				
				lengthVec *= 1.2f;
				const sm::Vector3 moveVec = ((p1normalAxis[indexForLowestVec] * lengthVec) / 2.0f);
				if((p2transform->position - (p1transform->position + moveVec)).Length() > (p2transform->position - p1transform->position).Length())
				{
					p1transform->position += (moveVec);
				}
				else
				{
					p1transform->position += (moveVec * -1.0f);
				}
			}
		});

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
