#include "Game.h"

using namespace std::placeholders;

Game::Game()
	: m_client(std::bind(&Game::CheckIncoming, this, _1), std::bind(&Game::OnClientDisconnect, this))
	, Engine()
{
	this->m_localPID = -1;
	this->m_gameID = -1;
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

		// TODO MAKE THIS BETTER
		if (m_gameID != UINT32_MAX && GetCurrentScene()->GetCurrentCamera()->GetCameraType() == CAMERATYPE::PLAY)
		{
			message<GameMsg> msg;
			msg.header.id = GameMsg::Game_MovePlayer;
			int8_t x = InputSystem::Get().GetAxis(Axis::HORIZONTAL);
			int8_t y = InputSystem::Get().GetAxis(Axis::VERTICAL);


			msg << this->m_localPID << m_gameID << x << y;

			m_client.Send(msg);
		}
	}
}

bool Game::OnStartup()
{
	//TODO: Remove grid stuff
	GridSystem grid;
	grid.Initialize();

	Scene& mainMenuScene = GetScene("MainMenu");
	mainMenuScene.on<ESceneUpdate>([](const ESceneUpdate& e, HeadlessScene& scene)
		{

			IMGUI(
				ImGui::Begin("Scene");
				ImGui::Text("MainMenu");
				ImGui::End();
			);
		});

	Scene& lobbyScene = GetScene("Lobby");
	lobbyScene.on<ESceneUpdate>([](const ESceneUpdate& e, HeadlessScene& scene) 
		{
			IMGUI(
				ImGui::Begin("Scene");
				ImGui::Text("Lobby");
				ImGui::End();
			);
		});

	Scene& gameScene = GetScene("Game");
	gameScene.on<ESceneUpdate>([](const ESceneUpdate& e, HeadlessScene& scene)
		{
			IMGUI(
				ImGui::Begin("Scene");
				ImGui::Text("Game");
				ImGui::End();
			);

		});

	SetScene(mainMenuScene);
	
	return true;
}

void Game::OnUserUpdate(float deltaTime)
{
	static float pingCheck = 0.f;
	IMGUI(
		ImGui::Begin("Network");

	if (m_client.IsConnected())
	{
		if (m_client.m_latency > 0)
		{
			ImGui::Text(std::string("Latency: " + std::to_string(m_client.m_latency) + "ms").c_str());
		}
		else
		{
			ImGui::Text(std::string("Latency: <1 ms").c_str());
		}

		if (m_gameID == UINT32_MAX)
		{
			if (ImGui::Button("Create Lobby"))
			{
				this->CreateLobby();
			}
			static uint32_t lobbyID = 0;
			ImGui::InputInt("LobbyID", (int*)&lobbyID);
			ImGui::SameLine();

			if (ImGui::Button("Join"))
			{
				this->JoinLobby(lobbyID);
			}
		}
		else
		{
			ImGui::Text(std::string("Game ID: " + std::to_string(m_gameID)).c_str());

			if (ImGui::Button("Leave Game"))
			{
				// TODO
			}
		}
		if (ImGui::Button("Disconnect"))
		{
			this->m_client.Disconnect();
		}
	}
	else
	{
		static char buffer[IPV6_ADDRSTRLEN];
		strcpy(buffer, "127.0.0.1");
		ImGui::InputText("IP", buffer, IPV6_ADDRSTRLEN);
		static uint16_t port = 4950;
		ImGui::InputInt("Port", (int*)&port);
		if (ImGui::Button("Connect"))
		{
			m_client.Connect(buffer, port);
		}
	}
	ImGui::End();
	);
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
		uint32_t count;
		msg >> count;
		
		std::unordered_map<uint32_t, comp::Transform> transforms;
		std::set<uint32_t> found;

		for (uint32_t i = 0; i < count; i++)
		{
			uint32_t entityID;
			comp::Transform t;
			msg >> entityID >> t;
			transforms[entityID] = t;
		}
		// TODO MAKE BETTER
		// Update entities with new transforms
		m_demoScene->GetScene().ForEachComponent<comp::Network, comp::Transform>([&](comp::Network& n, comp::Transform& t)
			{
				if (transforms.find(n.id) != transforms.end())
				{
					t = transforms.at(n.id);
					found.insert(n.id);
				}
			});

		// create new Entities
		for (const auto& t : transforms) {
			if (found.find(t.first) == found.end())
			{
				Entity entity = this->m_demoScene->CreatePlayerEntity(t.first);
				*entity.GetComponent<comp::Transform>() = t.second;
				
				if (m_localPID == t.first)
				{

					m_demoScene->GetScene().ForEachComponent<comp::Tag<CAMERA>>([&](Entity e, comp::Tag<CAMERA>& t)
						{
							comp::Camera3D* c = e.GetComponent<comp::Camera3D>();
							if (c)
							{
								if (c->camera.GetCameraType() == CAMERATYPE::PLAY)
								{
									m_demoScene->GetScene().SetCurrentCamera(&c->camera);
									c->camera.SetFollowTransform(entity.GetComponent<comp::Transform>());
								}
							}
						});
				}
			}
		}

		break;
	}
	case GameMsg::Game_AddPlayer:
	{
		uint32_t count; // Could be more than one player
		msg >> count;
		for (uint32_t i = 0; i < count; i++)
		{
			uint32_t remotePlayerID;
			msg >> remotePlayerID;
			LOG_INFO("Player with ID: %ld has joined the game!", remotePlayerID);
			Entity e = m_demoScene->CreatePlayerEntity(remotePlayerID);
			
		}

		break;
	}
	case GameMsg::Lobby_Accepted:
	{
		msg >> m_gameID;
		SetScene(m_demoScene->GetScene());
		LOG_INFO("You are now in lobby: %lu", m_gameID);
		break;
	}
	case GameMsg::Lobby_Invalid:
	{
		LOG_WARNING("Request denied: Invalid lobby");
		break;
	}
	case GameMsg::Game_RemovePlayer:
	{
		uint32_t playerID;
		msg >> playerID;

		// TODO Remove the entity of the player that matches ID

		m_demoScene->GetScene().ForEachComponent<comp::Network>([playerID](Entity& e, comp::Network& net)
			{
 				if (playerID == net.id)
				{
					e.Destroy();
				}
			}
		);
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

	m_demoScene->GetScene().ForEachComponent<comp::Network>([](Entity& e, comp::Network& net)
		{
			e.Destroy();
		}
	);

	SetScene("MainMenu");

	LOG_INFO("Disconnected from server!");
}

void Game::OnShutdown()
{

}
