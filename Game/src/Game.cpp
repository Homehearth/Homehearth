#include "Game.h"
#include "Button.h"
#include "TextField.h"
#include <DemoScene.h>
#include "Healthbar.h"

using namespace std::placeholders;

Game::Game()
	: m_client(std::bind(&Game::CheckIncoming, this, _1), std::bind(&Game::OnClientDisconnect, this))
	, Engine()
{
	this->m_localPID = -1;
	this->m_gameID = -1;
	this->m_predictionThreshhold = 0.001f;
}

Game::~Game()
{
	if (m_client.IsConnected())
	{
		m_client.Disconnect();
	}
}

void Game::UpdateNetwork(float deltaTime)
{
	//static float pingCheck = 0.f;
	//const float TARGET_PING_TIME = 5.0f;
	if (m_client.IsConnected())
	{
		m_client.Update();

		//pingCheck += deltaTime;

		//if (pingCheck > TARGET_PING_TIME)
		//{
		//	this->PingServer();
		//	pingCheck -= TARGET_PING_TIME;
		//}

		if (GetCurrentScene() == &GetScene("Game"))
		{
			if (GetCurrentScene()->GetCurrentCamera()->GetCameraType() == CAMERATYPE::PLAY)
			{
				
				message<GameMsg> msg;
				msg.header.id = GameMsg::Game_PlayerInput;

				
				msg << this->m_localPID << m_gameID << m_inputState;

				m_client.Send(msg);

				//reset input
				m_inputState.leftMouse = false;
			}
		}
	}
}

bool Game::OnStartup()
{
	// Scene logic
	sceneHelp::CreateLobbyScene(this);
	sceneHelp::CreateGameScene(this);
	sceneHelp::CreateMainMenuScene(this);
	sceneHelp::CreateJoinLobbyScene(this);
	sceneHelp::CreateLoadingScene(this);

	// Set Current Scene
	SetScene("MainMenu");

	return true;
}

void Game::OnUserUpdate(float deltaTime)
{
	static float pingCheck = 0.f;


	/*
if (GetCurrentScene() == &GetScene("Game") && GetCurrentScene()->GetCurrentCamera()->GetCameraType() == CAMERATYPE::PLAY)
{
	if (m_players.find(m_localPID) != m_players.end())
	{
		comp::Transform* t = m_players.at(m_localPID).GetComponent<comp::Transform>();

		int x = InputSystem::Get().GetAxis(Axis::HORIZONTAL);
		int z = InputSystem::Get().GetAxis(Axis::VERTICAL);
		if (x || z)
		{
			t->position.x += 10.f * deltaTime * x;
			t->position.z += 10.f * deltaTime * z;

			predictedPositions.push_back(*t);
		}

		//LOG_INFO("Predicted size: %llu", predictedPositions.size());
		if (sm::Vector3::Distance(t->position, test.position) > m_predictionThreshhold)
		{
			t->position.x = test.position.x;
			t->position.z = test.position.z;
		}
	}
}
		*/

		//Update InputState
	this->UpdateInput();

}


void Game::OnShutdown()
{
	m_players.clear();
}


void Game::CheckIncoming(message<GameMsg>& msg)
{
	switch (msg.header.id)
	{
	case GameMsg::Client_Accepted:
	{
		LOG_INFO("You are validated!");
		break;
	}
	case GameMsg::Server_GetPing:
	{
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
		m_client.m_latency = int(std::chrono::duration<double>(timeNow - this->m_timeThen).count() * 1000);
		break;
	}
	case GameMsg::Server_AssignID:
	{
		msg >> this->m_localPID;

		LOG_INFO("YOUR ID IS: %lu", this->m_localPID);
		break;
	}
	case GameMsg::Game_Snapshot:
	{
		uint32_t currentTick;
		msg >> currentTick;
		uint32_t count;
		msg >> count;

		for (int i = 0; i < count; i++)
		{
			uint32_t entityID;
			msg >> entityID;

			Entity entity;
			if (m_gameEntities.find(entityID) != m_gameEntities.end())
			{
				entity = m_gameEntities.at(entityID);
			}
			else {
				LOG_WARNING("Updating: Entity %u not in m_gameEntities, should not happen...", entityID);
				continue;
			}

			uint32_t bits;
			msg >> bits;
			std::bitset<ecs::Component::COMPONENT_MAX> compSet(bits);

			for (int j = ecs::Component::COMPONENT_COUNT; j >= 0; j--)
			{
				if (compSet.test(j))
				{
					switch (j)
					{
					case ecs::Component::TRANSFORM:
					{
						comp::Transform t;
						msg >> t;
						entity.AddComponent<comp::Transform>(t);

						break;
					}
					case ecs::Component::HEALTH:
					{
						comp::Health h;
						msg >> h;
						entity.AddComponent<comp::Health>(h);
						break;
					}
					case ecs::Component::BOUNDING_ORIENTED_BOX:
					{
						comp::BoundingOrientedBox b;
						msg >> b;
						entity.AddComponent<comp::BoundingOrientedBox>(b);
						break;
					}
					default:
						break;
					}
				}
			}
		}

		break;
	}
	case GameMsg::Game_AddEntity:
	{
		uint32_t count; // Could be more than one Entity
		msg >> count;
		LOG_INFO("Received %u entities", count);

		for (uint32_t i = 0; i < count; i++)
		{
			Entity e = CreateEntityFromMessage(msg);
			m_gameEntities.insert(std::make_pair(e.GetComponent<comp::Network>()->id, e));
			if (e.GetComponent<comp::Network>()->id == m_localPID)
			{
				LOG_INFO("You added yourself, congratulations!");
				m_players[m_localPID] = e;

				GetScene("Game").ForEachComponent<comp::Tag<TagType::CAMERA>>([&](Entity entt, comp::Tag<TagType::CAMERA>& t)
					{
						comp::Camera3D* c = entt.GetComponent<comp::Camera3D>();
						if (c)
						{
							c->camera.SetFollowEntity(e);
						}
					});
			}
			else if (e.GetComponent<comp::Player>())
			{
				LOG_INFO("A remote player added!");
				m_players[e.GetComponent<comp::Network>()->id] = e;
			}
			// TODO DEBUG
#ifdef  _DEBUG
			comp::Tile* tile = e.GetComponent<comp::Tile>();
			if (tile)
			{
				if (tile->type == TileType::EMPTY)
				{
					comp::Renderable* renderable = e.AddComponent<comp::Renderable>();
					renderable->model = ResourceManager::Get().GetResource<RModel>("Plane1.obj");
					renderable->model->ChangeMaterial("TileEmpty.mtl");
				}
				else if (tile->type == TileType::BUILDING || tile->type == TileType::UNPLACABLE)
				{
					comp::Renderable* renderable = e.AddComponent<comp::Renderable>();
					renderable->model = ResourceManager::Get().GetResource<RModel>("Plane2.obj");
					renderable->model->ChangeMaterial("TileBuilding.mtl");
				}
				
			}
#endif //  _DEBUG


		}

		LOG_INFO("Successfully loaded all entities!");

		if (GetCurrentScene() == &GetScene("Loading"))
		{
			SetScene("Lobby");
		}

		break;
	}
	case GameMsg::Game_RemoveEntity:
	{
		uint32_t count;
		msg >> count;
		for (uint32_t i = 0; i < count; i++)
		{
			uint32_t id;
			msg >> id;
			if (m_gameEntities.find(id) != m_gameEntities.end())
			{
				m_gameEntities.at(id).Destroy();
				m_gameEntities.erase(id);
			}
			// Was the entity a player?
			if (m_players.find(id) != m_players.end())
			{
				m_players.at(id).Destroy();
				m_players.erase(id);
			}

		}
		LOG_INFO("Removed %u entities", count);
		break;
	}
	case GameMsg::Lobby_Accepted:
	{
		msg >> m_gameID;
		this->SetScene("Loading");
		LOG_INFO("You are now in lobby: %lu", m_gameID);

		break;
	}
	case GameMsg::Lobby_Invalid:
	{
		std::string err;
		msg >> err;
		SetScene("JoinLobby");
		LOG_WARNING("%s", err.c_str());
		break;
	}
	case GameMsg::Lobby_AcceptedLeave:
	{
		LOG_WARNING("Left Lobby %u", m_gameID);
		m_gameID = -1;
		SetScene("JoinLobby");
		break;
	}
	case GameMsg::Game_Start:
	{
		SetScene("Game");
		break;
	}
	case GameMsg::Lobby_Update:
	{
		uint32_t count;
		msg >> count;
		const uint32_t* spots = &count;
		std::string ids[MAX_PLAYERS_PER_LOBBY];

		for (uint32_t i = 0; i < count; i++)
		{
			std::string playerPlate;
			uint32_t playerID;
			msg >> playerPlate;
			msg >> playerID;
			ids[i] = playerPlate;

			if (m_players.find(playerID) != m_players.end())
			{
				m_players.at(playerID).GetComponent<comp::NamePlate>()->namePlate = playerPlate;
				dynamic_cast<rtd::Text*>(GetScene("Lobby").GetCollection("playerIcon" + std::to_string(i + 1))->elements[1].get())->SetText(playerPlate);
			}
		}

		dynamic_cast<rtd::Text*>(GetScene("Lobby").GetCollection("LobbyDesc")->elements[1].get())->SetText("Lobby ID: " + std::to_string(m_gameID));

		for (int i = 0; i < MAX_PLAYERS_PER_LOBBY; i++)
		{
			GetScene("Lobby").GetCollection("playerIcon" + std::to_string(i + 1))->Hide();
		}
		for (uint32_t i = 0; i < count; i++)
		{
			GetScene("Lobby").GetCollection("playerIcon" + std::to_string(i + 1))->Show();
		}

		Scene& gameScene = GetScene("Game");
		// Map healthbars to players.
		GameSystems::UpdateHealthbar(gameScene);

		for (size_t i = 0; i < count; i++)
		{
			Collection2D* collect = gameScene.GetCollection("player" + std::to_string(i + 1) + "Info");
			collect->Show();
			dynamic_cast<rtd::Text*>(collect->elements[1].get())->SetText(ids[i]);
		}

		break;
	}
	}
}

void Game::PingServer()
{
	message<GameMsg> msg = {};
	msg.header.id = GameMsg::Server_GetPing;
	msg << this->m_localPID;

	this->m_timeThen = std::chrono::system_clock::now();
	m_client.Send(msg);
}

void Game::JoinLobby(uint32_t lobbyID)
{
	if (m_gameID == (uint32_t)-1)
	{
		message<GameMsg> msg;
		msg.header.id = GameMsg::Lobby_Join;
		msg << m_playerName;
		msg << this->m_localPID << lobbyID;
		m_client.Send(msg);
	}
	else
	{
		LOG_WARNING("Request denied: You are already in a lobby");
	}
}

void Game::CreateLobby()
{
	if (m_gameID == (uint32_t)-1)
	{
		message<GameMsg> msg;

		msg.header.id = GameMsg::Lobby_Create;
		msg << m_playerName;
		msg << this->m_localPID;
		m_client.Send(msg);
	}
	else
	{
		LOG_WARNING("Request denied: You are already in a lobby");
	}
}

void Game::OnClientDisconnect()
{
	this->m_gameID = -1;
	this->m_localPID = -1;

	// remove all network entities
	GetScene("Game").ForEachComponent<comp::Network>([](Entity& e, comp::Network& net)
		{
			e.Destroy();
		}
	);

	SetScene("MainMenu");

	LOG_INFO("Disconnected from server!");
}

void Game::SendStartGame()
{
	network::message<GameMsg> msg;
	msg.header.id = GameMsg::Game_PlayerReady;
	msg << m_localPID << m_gameID;
	m_client.Send(msg);
}

Entity Game::CreateEntityFromMessage(message<GameMsg>& msg)
{
	Entity e = GetScene("Game").CreateEntity();
	uint32_t bits;
	msg >> bits;
	std::bitset<ecs::Component::COMPONENT_MAX> compSet(bits);

	for (int i = ecs::Component::COMPONENT_COUNT - 1; i >= 0; i--)
	{
		if (compSet.test(i))
		{
			switch (i)
			{
			case ecs::Component::NETWORK:
			{
				comp::Network n;
				msg >> n;
				*e.AddComponent<comp::Network>() = n;
				break;
			}
			case ecs::Component::TRANSFORM:
			{
				comp::Transform t;
				msg >> t;
				t.rotation.Normalize();
				*e.AddComponent<comp::Transform>() = t;
				break;
			}
			case ecs::Component::VELOCITY:
			{
				comp::Velocity v;
				msg >> v;
				*e.AddComponent<comp::Velocity>() = v;
				break;
			}
			case ecs::Component::MESH_NAME:
			{
				std::string name;
				msg >> name;
				e.AddComponent<comp::Renderable>()->model = ResourceManager::Get().GetResource<RModel>(name);
				break;
			}
			case ecs::Component::NAME_PLATE:
			{
				std::string name;
				msg >> name;
				e.AddComponent<comp::NamePlate>()->namePlate = name;
				break;
			}
			case ecs::Component::HEALTH:
			{
				comp::Health heal;
				msg >> heal.maxHealth;
				msg >> heal.isAlive;
				msg >> heal.currentHealth;
				*e.AddComponent<comp::Health>() = heal;
				break;
			}
			case ecs::Component::BOUNDING_ORIENTED_BOX:
			{
				comp::BoundingOrientedBox box;
				msg >> box;
				*e.AddComponent<comp::BoundingOrientedBox>() = box;
				break;
			}
			case ecs::Component::BOUNDING_SPHERE:
			{
				comp::BoundingSphere s;
				msg >> s;
				*e.AddComponent<comp::BoundingSphere>() = s;
				break;
			}
			case ecs::Component::LIGHT:
			{
				comp::Light l;
				msg >> l;
				*e.AddComponent<comp::Light>() = l;
				break;
			}
			case ecs::Component::PLAYER:
			{
				comp::Player p;
				msg >> p;
				*e.AddComponent<comp::Player>() = p;
				break;
			}
			case ecs::Component::TILE:
			{
				comp::Tile t;
				msg >> t;
				*e.AddComponent<comp::Tile>() = t;
				break;
			}
			default:
				LOG_WARNING("Retrieved unimplemented component %u", i)
					break;
			}
		}
	}


	return e;
}

void Game::UpdateInput()
{
	m_inputState.axisHorizontal = InputSystem::Get().GetAxis(Axis::HORIZONTAL);
	m_inputState.axisVertical = InputSystem::Get().GetAxis(Axis::VERTICAL);
	if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::HELD))
	{
		m_inputState.leftMouse = true;
		m_inputState.mouseRay = InputSystem::Get().GetMouseRay();
	}
}

