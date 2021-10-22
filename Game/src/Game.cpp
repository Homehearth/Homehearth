#include "Game.h"

using namespace std::placeholders;

Game::Game()
	: m_client(std::bind(&Game::CheckIncoming, this, _1), std::bind(&Game::OnClientDisconnect, this))
	, Engine()
{
	this->m_localPID = -1;
	this->m_gameID = -1;
	this->m_isLeavingLobby = false;
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
		if (m_gameID != UINT32_MAX && GetCurrentScene()->GetCurrentCamera()->GetCameraType() == CAMERATYPE::PLAY && !m_isLeavingLobby)
		{
			int8_t x = InputSystem::Get().GetAxis(Axis::HORIZONTAL);
			int8_t z = InputSystem::Get().GetAxis(Axis::VERTICAL);

			GetScene("Game").ForEachComponent<comp::Transform, comp::Tag<TagType::LOCAL_PLAYER>>([&]
			(comp::Transform& t, comp::Tag<TagType::LOCAL_PLAYER>& tag)
				{
					t.position.x += x * 10.f * deltaTime;
					t.position.z += z * 10.f * deltaTime;
				}
			);
			message<GameMsg> msg;
			msg.header.id = GameMsg::Game_PlayerInput;

			msg << this->m_localPID << m_gameID << x << z;

			m_client.Send(msg);
		}
	}
}

bool Game::OnStartup()
{
	// Scene logic
	sceneHelp::CreateLobbyScene(*this);
	rtd::Handler2D::Get().SetVisibilityAll(false);
	sceneHelp::CreateMainMenuScene(*this);
	sceneHelp::CreateGameScene(*this);


	// Set Current Scene
	SetScene("MainMenu");

	return true;
}

void Game::OnUserUpdate(float deltaTime)
{
	static float pingCheck = 0.f;

#if RENDER_IMGUI == 0
	// Connect Screen
	rtd::TextField* port_text = GET_ELEMENT("portBuffer", rtd::TextField);
	rtd::TextField* ip_text = GET_ELEMENT("ipBuffer", rtd::TextField);
	if (ip_text && port_text)
	{
		if (ip_text->IsHovered())
		{
			ip_text->GetBorder()->SetColor({ 1.0f, 1.0f, 0.0f, 1.0f });
		}
		else
		{
			ip_text->GetBorder()->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });
		}
		std::string* ipBuffer = nullptr;
		std::string* portBuffer = nullptr;
		ip_text->GetBuffer(ipBuffer);
		port_text->GetBuffer(portBuffer);

		if (ip_text->GetIsUsed() || port_text->GetIsUsed())
		{
			if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::Enter, KeyState::PRESSED))
			{
				ip_text->SetIsUsed(false);
				port_text->SetIsUsed(false);

				int port = 0;
				try {
					port = std::stoi(*portBuffer);
				}
				catch (const std::exception& e) {
					LOG_ERROR("Invalid port");
				}
				if (!portBuffer->empty() && !ipBuffer->empty() && m_client.Connect(ipBuffer->c_str(), port))
				{
					rtd::Handler2D::SetVisibilityAll(false);
					GET_ELEMENT("welcome_text", rtd::Text)->SetVisibility(true);
					GET_ELEMENT("gameInfoText", rtd::Text)->SetVisibility(true);
					GET_ELEMENT("joinButton", rtd::Button)->SetVisibility(true);
					GET_ELEMENT("hostButton", rtd::Button)->SetVisibility(true);
					GET_ELEMENT("lobbyBuffer", rtd::TextField)->SetVisibility(true);
					GET_ELEMENT("exitGameButton", rtd::Button)->SetVisibility(true);
				}
			}
		}
		
	}

	rtd::Button* connectButton = GET_ELEMENT("connectButton", rtd::Button);
	if (connectButton)
	{
		if (connectButton->IsClicked())
		{
			std::string* ipBuffer = nullptr;
			std::string* portBuffer = nullptr;

			ip_text->GetBuffer(ipBuffer);
			ip_text->SetIsUsed(false);

			port_text->GetBuffer(portBuffer);
			port_text->SetIsUsed(false);

			int port = 0;
			try {
				port = std::stoi(*portBuffer);
			}
			catch (const std::exception& e) {
				LOG_ERROR("Invalid port");
			}

			if (m_client.Connect(ipBuffer->c_str(), port))
			{
				rtd::Handler2D::SetVisibilityAll(false);
				GET_ELEMENT("welcome_text", rtd::Text)->SetVisibility(true);
				GET_ELEMENT("gameInfoText", rtd::Text)->SetVisibility(true);
				GET_ELEMENT("joinButton", rtd::Button)->SetVisibility(true);
				GET_ELEMENT("hostButton", rtd::Button)->SetVisibility(true);
				GET_ELEMENT("lobbyBuffer", rtd::TextField)->SetVisibility(true);
				GET_ELEMENT("exitGameButton", rtd::Button)->SetVisibility(true);

			}
		}
	}
	
	// Main Menu Screen
	rtd::Button* exit_button = GET_ELEMENT("exitGameButton", rtd::Button);
	if (exit_button)
	{
		if (exit_button->IsClicked())
		{
			Shutdown();
		}
	}

	if (m_client.IsConnected())
	{
		rtd::Button* hostButton = GET_ELEMENT("hostButton", rtd::Button);
		if (hostButton)
		{
			if (hostButton->IsClicked())
			{
				this->CreateLobby();
			}
		}

		std::string* lobbyBuffer = nullptr;
		rtd::TextField* lobby_text = GET_ELEMENT("lobbyBuffer", rtd::TextField);
		if (lobby_text)
		{
			lobby_text->GetBuffer(lobbyBuffer);
		}

		rtd::Button* joinButton = GET_ELEMENT("joinButton", rtd::Button);
		if (joinButton)
		{
			if (joinButton->IsClicked())
			{
				int lobbyID = 0;
				if (lobbyBuffer)
				{
					try {
						lobbyID = std::stoi(*lobbyBuffer);
					}
					catch (const std::exception& e) {
						LOG_ERROR("Invalid lobby ID");
					}
				}
				this->JoinLobby(lobbyID);

			}
		}

		

		
	}
#endif

	IMGUI(
		ImGui::Begin("Network");

	if (m_client.IsConnected())
	{
		ImGui::Text(std::string("Local Client ID: " + std::to_string(m_localPID)).c_str());

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

			if (m_isLeavingLobby)
			{
				ImGui::BeginDisabled();
				ImGui::Button("Leave Game");
				ImGui::EndDisabled();

			}
			else if (ImGui::Button("Leave Game"))
			{
				m_isLeavingLobby = true;
				message<GameMsg> msg;
				msg.header.id = GameMsg::Lobby_Leave;
				msg << m_localPID << m_gameID;
				m_client.Send(msg);
			}

		}
		if (ImGui::Button("Disconnect"))
		{
			this->m_client.Disconnect();
		}
	}
	else
	{
		static char buffer[IPV6_ADDRSTRLEN] = "127.0.0.1";
		//strcpy(buffer, "127.0.0.1");
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


void Game::OnShutdown()
{

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

		for (uint32_t i = 0; i < count; i++)
		{
			uint32_t entityID;
			comp::Transform t;
			msg >> entityID >> t;
			transforms[entityID] = t;
		}
		// TODO MAKE BETTER
		// Update entities with new transforms
		GetScene("Game").ForEachComponent<comp::Network, comp::Transform>([&](comp::Network& n, comp::Transform& t)
			{
				if (transforms.find(n.id) != transforms.end())
				{
					t = transforms.at(n.id);
				}

			});

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

			LOG_INFO("Added entity %u", e.GetComponent<comp::Network>()->id);

			if (e.GetComponent<comp::Network>()->id == m_localPID)
			{
				LOG_INFO("This player added");
				e.AddComponent<comp::Tag<TagType::LOCAL_PLAYER>>();

				GetScene("Game").ForEachComponent<comp::Tag<TagType::CAMERA>>([&](Entity entt, comp::Tag<TagType::CAMERA>& t)
					{
						comp::Camera3D* c = entt.GetComponent<comp::Camera3D>();
						if (c)
						{
							c->camera.SetFollowEntity(e);
						}
					});
			}
		}

		break;
	}
	case GameMsg::Game_RemoveEntity:
	{
		uint32_t count;
		msg >> count;
		std::vector<uint32_t> ids(count);
		for (uint32_t i = 0; i < count; i++)
		{
			msg >> ids[i];
		}

		GetScene("Game").ForEachComponent<comp::Network>([&](Entity& e, comp::Network& net)
			{
				if (std::find(ids.begin(), ids.end(), net.id) != ids.end())
				{
					LOG_INFO("Removed Entity %u", net.id);
					e.Destroy();
				}
			});

		break;
	}
	case GameMsg::Lobby_Accepted:
	{
		msg >> m_gameID;
		SetScene("Game");
		LOG_INFO("You are now in lobby: %lu", m_gameID);
		break;
	}
	case GameMsg::Lobby_Invalid:
	{
		std::string err;
		msg >> err;
		LOG_WARNING("Request denied: %s", err.c_str());
		break;
	}
	case GameMsg::Lobby_AcceptedLeave:
	{
		LOG_WARNING("Left Lobby %u", m_gameID);
		m_isLeavingLobby = false;
		m_gameID = -1;
		SetScene("MainMenu");
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

	// remove all network entities
	GetScene("Game").ForEachComponent<comp::Network>([](Entity& e, comp::Network& net)
		{
			LOG_INFO("Removed entity %u on disconnect", net.id);
			e.Destroy();
		}
	);

	SetScene("MainMenu");

	LOG_INFO("Disconnected from server!");
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
				uint32_t id;
				msg >> id;
				e.AddComponent<comp::Network>()->id = id;
				break;
			}
			case ecs::Component::TRANSFORM:
			{
				comp::Transform t;
				msg >> t;
				*e.AddComponent<comp::Transform>() = t;
				break;
			}
			case ecs::Component::MESH_NAME:
			{
				std::string name;
				msg >> name;
				e.AddComponent<comp::Renderable>()->model = ResourceManager::Get().GetResource<RModel>(name);
				break;
			}
			case ecs::Component::BOUNDING_ORIENTED_BOX:
			{
				comp::BoundingOrientedBox box;
				msg >> box.Orientation >> box.Extents >> box.Center;
				*e.AddComponent<comp::BoundingOrientedBox>() = box;
				break;
			}
			case ecs::Component::BOUNDING_SPHERE:
			{
				comp::BoundingSphere s;
				msg >> s.Radius >> s.Center;
				*e.AddComponent<comp::BoundingSphere>() = s;
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

