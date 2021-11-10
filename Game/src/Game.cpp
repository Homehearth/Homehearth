#include "Game.h"
#include "Button.h"
#include "TextField.h"
#include <SceneHelper.h>
#include "Healthbar.h"

using namespace std::placeholders;

Game::Game()
	: m_client(std::bind(&Game::CheckIncoming, this, _1), std::bind(&Game::OnClientDisconnect, this))
	, Engine()
{
	this->m_localPID = -1;
	this->m_gameID = -1;
	this->m_predictionThreshhold = 0.001f;
	this->m_waveTimer = 0;
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
	static float pingCheck = 0.f;
	const float TARGET_PING_TIME = 5.0f;
	if (m_client.IsConnected())
	{
		m_client.Update();

		pingCheck += deltaTime;

		if (pingCheck > TARGET_PING_TIME)
		{
			this->PingServer();
			pingCheck -= TARGET_PING_TIME;
		}

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
				m_inputState.rightMouse = false;
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

	this->LoadMapColliders("AllBounds.fbx");
	// Set Current Scene
	SetScene("MainMenu");


	return true;
}

void Game::OnUserUpdate(float deltaTime)
{
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
	if (GetCurrentScene() == &GetScene("Game"))
	{
		// Re-enable when we have the transparency pass done
		//GameSystems::CheckLOS(GetScene("Game"), m_players.at(m_localPID).GetComponent<comp::Transform>()->position, m_LOSColliders);
		this->UpdateInput();
	}
}


void Game::OnShutdown()
{
	m_players.clear();
	m_mapEntity.Destroy();
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

		for (uint32_t i = 0; i < count; i++)
		{
			uint32_t entityID;
			msg >> entityID;

			Entity entity;
			if (m_gameEntities.find(entityID) != m_gameEntities.end())
			{
				entity = m_gameEntities.at(entityID);
				UpdateEntityFromMessage(entity, msg);
			}
			else {
				LOG_WARNING("Updating: Entity %u not in m_gameEntities, should not happen...", entityID);

			}
		}

		break;
	}
	case GameMsg::Game_UpdateComponent:
	{
		uint32_t currentTick;
		msg >> currentTick;
		uint32_t count; // Could be more than one Entity
		msg >> count;

		for (uint32_t i = 0; i < count; i++)
		{
			uint32_t entityID;
			msg >> entityID;

			Entity entity;
			if (m_gameEntities.find(entityID) != m_gameEntities.end())
			{
				entity = m_gameEntities.at(entityID);
				UpdateEntityFromMessage(entity, msg);
			}
			else {
				LOG_WARNING("Updating: Entity %u not in m_gameEntities, should not happen...", entityID);

			}
		}

		break;
	}
	case GameMsg::Game_AddEntity:
	{
		uint32_t currentTick;
		msg >> currentTick;
		uint32_t count; // Could be more than one Entity
		msg >> count;
#ifdef _DEBUG
		LOG_INFO("Received %u entities", count);
#endif

		for (uint32_t i = 0; i < count; i++)
		{
			Entity e = GetScene("Game").CreateEntity();
			uint32_t entityID;
			msg >> entityID;
			e.AddComponent<comp::Network>(entityID);
			UpdateEntityFromMessage(e, msg);

			m_gameEntities.insert(std::make_pair(entityID, e));
			if (e.GetComponent<comp::Network>()->id == m_localPID)
			{
#ifdef _DEBUG
				LOG_INFO("You added yourself, congratulations!");
#endif
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

		}

		if (GetCurrentScene() == &GetScene("Loading"))
		{
			SetScene("Lobby");
		}
#ifdef _DEBUG
		LOG_INFO("Successfully loaded all entities!");
#endif

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
#ifdef _DEBUG
		LOG_INFO("Removed %u entities", count);
#endif
		break;
	}
	case GameMsg::Game_BackToLobby:
	{
		auto it = m_players.begin();
		while (it != m_players.end())
		{
			it->second.GetComponent<comp::Player>()->isReady = false;
			it++;
		}
		SetScene("Lobby");
		break;
	}
	case GameMsg::Lobby_Accepted:
	{
		msg >> m_gameID;
		this->SetScene("Loading");
		this->LoadAllAssets();

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
	case GameMsg::Game_WaveTimer:
	{
		msg >> m_waveTimer;
		Element2D* elem = GetScene("Game").GetCollection("timer")->elements[0].get();
		if (elem)
		{
			if (m_waveTimer > 0)
				dynamic_cast<rtd::Text*>(elem)->SetText("\nUntil next Wave:\n" + std::to_string(m_waveTimer));
			else
				dynamic_cast<rtd::Text*>(elem)->SetText("\nUnder Attack!");
		}
		break;
	}
	case GameMsg::Lobby_Update:
	{
		uint32_t count;
		msg >> count;
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
				rtd::Text* plT = dynamic_cast<rtd::Text*>(GetScene("Game").GetCollection("dynamicPlayer" + std::to_string(i + 1) + "namePlate")->elements[0].get());
				if (plT)
				{
					plT->SetText(playerPlate);
					plT->SetStretch(D2D1Core::GetDefaultFontSize() * plT->GetText().length(), D2D1Core::GetDefaultFontSize());
				}
			}
		}
		if (m_players.find(m_localPID) != m_players.end())
		{
			comp::Player* player = m_players.at(m_localPID).GetComponent<comp::Player>();
			rtd::Text* readyText = dynamic_cast<rtd::Text*>(GetScene("Lobby").GetCollection("StartGame")->elements[1].get());
			if (readyText)
			{
				if (player->isReady)
				{
					readyText->SetText("Ready");
				}
				else
				{
					readyText->SetText("Not ready");
				}
			}
		}

		dynamic_cast<rtd::Text*>(GetScene("Lobby").GetCollection("LobbyDesc")->elements[1].get())->SetText("Lobby ID: " + std::to_string(m_gameID));


		for (uint32_t i = 0; i < count; i++)
		{
			GetScene("Lobby").GetCollection("playerIcon" + std::to_string(i + 1))->Show();
		}
		for (uint32_t i = count; i < MAX_PLAYERS_PER_LOBBY; i++)
		{
			GetScene("Lobby").GetCollection("playerIcon" + std::to_string(i + 1))->Hide();
		}

		Scene& gameScene = GetScene("Game");
		// Map healthbars to players.
		GameSystems::UpdateHealthbar(gameScene);
		break;

	}
	}
}
void Game::PingServer()
{
	message<GameMsg> msg = {};
	msg.header.id = GameMsg::Server_GetPing;
	msg << this->m_localPID;

	LOG_INFO("Pinging server!");

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

	m_client.m_qPrioMessagesIn.clear();
	m_client.m_qMessagesIn.clear();
	LOG_INFO("Disconnected from server!");
}

void Game::SendStartGame()
{
	network::message<GameMsg> msg;
	msg.header.id = GameMsg::Game_PlayerReady;
	msg << m_localPID << m_gameID;

	m_client.Send(msg);
}

Entity& Game::GetLocalPlayer()
{
	return this->m_players.at(m_localPID);
}

void Game::UpdateEntityFromMessage(Entity e, message<GameMsg>& msg)
{

	uint32_t bits;
	msg >> bits;
	std::bitset<ecs::Component::COMPONENT_MAX> compSet(bits);

	for (int i = ecs::Component::COMPONENT_COUNT - 1; i >= 0; i--)
	{
		if (compSet.test(i))
		{
			switch (i)
			{
			case ecs::Component::TRANSFORM:
			{
				comp::Transform t;
				msg >> t;
				t.rotation.Normalize();
				e.AddComponent<comp::Transform>(t);
				break;
			}
			case ecs::Component::VELOCITY:
			{
				comp::Velocity v;
				msg >> v;
				e.AddComponent<comp::Velocity>(v);
				break;
			}
			case ecs::Component::MESH_NAME:
			{
				std::string name;
				msg >> name;
				e.AddComponent<comp::Renderable>()->model = ResourceManager::Get().GetResource<RModel>(name);
				break;
			}
			case ecs::Component::ANIMATOR_NAME:
			{
				comp::AnimatorName animName;
				msg >> animName.name;

				//Get model
				comp::Renderable* renderable = e.GetComponent<comp::Renderable>();
				if (renderable && renderable->model)
				{

					//Add an animator if we can get it
					if (!animName.name.empty())
					{
						std::shared_ptr<RAnimator> anim = ResourceManager::Get().CopyResource<RAnimator>(animName.name, true);
						if (anim)
						{
							if (anim->LoadSkeleton(renderable->model->GetSkeleton()))
								e.AddComponent<comp::Animator>()->animator = anim;
						}
					}
					else
					{
						e.RemoveComponent<comp::Animator>();
					}
				}
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
				msg >> heal;
				e.AddComponent<comp::Health>(heal);
				break;
			}
			case ecs::Component::BOUNDING_ORIENTED_BOX:
			{
				comp::BoundingOrientedBox box;
				msg >> box;
				e.AddComponent<comp::BoundingOrientedBox>(box);
				break;
			}
			case ecs::Component::BOUNDING_SPHERE:
			{
				comp::BoundingSphere s;
				msg >> s;
				e.AddComponent<comp::BoundingSphere>(s);
				break;
			}
			case ecs::Component::LIGHT:
			{
				comp::Light l;
				msg >> l;
				e.AddComponent<comp::Light>(l);
				break;
			}
			case ecs::Component::PLAYER:
			{
				comp::Player p;
				msg >> p;
				e.AddComponent<comp::Player>(p);
				break;
			}
			default:
				LOG_WARNING("Retrieved unimplemented component %u", i);
				break;
			}
		}
	}

}

void Game::UpdateInput()
{
	m_inputState.axisHorizontal = InputSystem::Get().GetAxis(Axis::HORIZONTAL);
	m_inputState.axisVertical = InputSystem::Get().GetAxis(Axis::VERTICAL);
	if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::HELD))
	{
		m_inputState.leftMouse = true;
	}
	if (InputSystem::Get().CheckMouseKey(MouseKey::RIGHT, KeyState::PRESSED))
	{
		m_inputState.rightMouse = true;
	}
	m_inputState.mouseRay = InputSystem::Get().GetMouseRay();
}

void Game::LoadAllAssets()
{
	ResourceManager::Get().GetResource<RModel>("MonsterCharacter.fbx");
	ResourceManager::Get().GetResource<RModel>("Barrel.obj");
	ResourceManager::Get().GetResource<RModel>("Defence.obj");
	ResourceManager::Get().GetResource<RModel>("Plane1.obj");
	ResourceManager::Get().GetResource<RModel>("Knight.fbx");
	Entity e = GetScene("Game").CreateEntity();
	e.AddComponent<comp::Transform>();
	e.AddComponent<comp::Renderable>()->model = ResourceManager::Get().GetResource<RModel>("GameScene.obj");
}

bool Game::LoadMapColliders(const std::string& filename)
{
	std::string filepath = BOUNDSPATH + filename;
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile
	(
		filepath,
		aiProcess_JoinIdenticalVertices |
		aiProcess_ConvertToLeftHanded
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

		aiNode* node = scene->mRootNode->FindNode(mesh->mName);

		if (node)
		{
			aiVector3D pos;
			aiVector3D scl;
			aiQuaternion rot;
			node->mTransformation.Decompose(scl, rot, pos);

			dx::XMFLOAT3 center = { pos.x, pos.y, pos.z };
			dx::XMFLOAT3 extents = { scl.x / 2.f, scl.y / 2.f, scl.z / 2.f };
			dx::XMFLOAT4 orientation = { rot.x, rot.y, rot.z, rot.w };

			dx::BoundingOrientedBox bob(center, extents, orientation);

			// This is for renderside
			Entity collider = GetScene("Game").CreateEntity();
			collider.AddComponent<comp::BoundingOrientedBox>()->Center = center;
			collider.GetComponent<comp::BoundingOrientedBox>()->Extents = extents;
			collider.GetComponent<comp::BoundingOrientedBox>()->Orientation = orientation;
			collider.AddComponent<comp::Tag<TagType::STATIC>>();
			collider.AddComponent<comp::Tag<TagType::MAP_BOUNDS>>();
			m_LOSColliders.push_back(bob);
		}
	}

	return true;
}
