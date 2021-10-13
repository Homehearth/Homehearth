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
		if (m_gameID != UINT32_MAX)
		{
			message<GameMsg> msg;
			msg.header.id = GameMsg::Game_MovePlayer;
			int x = InputSystem::Get().GetAxis(Axis::HORIZONTAL);
			int y = InputSystem::Get().GetAxis(Axis::VERTICAL);
			msg << x << y;

			m_client.Send(msg);
		}
	}
}

bool Game::OnStartup()
{
	// Scene logic
	m_demoScene = std::make_unique<DemoScene>(*this);

	//Set as current scene
	SetScene(m_demoScene->GetScene());

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
			if (this->m_gameID == (uint32_t)-1 && m_localPID == remotePlayerID)
			{
				*m_demoScene->GetScene().m_currentCamera = m_demoScene->m_gameCamera;
				m_demoScene->m_gameCamera.SetFollowVelocity(e.GetComponent<comp::Velocity>());
			}
		}

		break;
	}
	case GameMsg::Lobby_Accepted:
	{
		msg >> m_gameID;
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
				if (playerID == net.key)
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
	LOG_INFO("Disconnected from server!");

	this->m_gameID = -1;
	this->m_localPID = -1;

	m_demoScene->GetScene().ForEachComponent<comp::Network>([](Entity& e, comp::Network& net)
		{
			e.Destroy();
		}
	);
}

void Game::OnShutdown()
{

}
