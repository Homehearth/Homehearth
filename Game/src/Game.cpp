#include "Game.h"

using namespace std::placeholders;

Game::Game()
	: m_client(std::bind(&Game::CheckIncoming, this, _1))
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

bool Game::OnStartup()
{
	// Scene logic
	m_demoScene = std::make_unique<DemoScene>(*this, m_client, &this->m_localPID, &this->m_gameID);

	//Set as current scene
	SetScene(m_demoScene->GetScene());

	return true;
}

void Game::OnUserUpdate(float deltaTime)
{
	m_demoScene->CameraUpdate(deltaTime);


	IMGUI(
		ImGui::Begin("Test");

	if (m_client.IsConnected())
	{
		if (ImGui::Button("Ping"))
		{
			PingServer();
		}
		if (ImGui::Button("Create Lobby"))
		{
			message<GameMsg> msg;

			msg.header.id = GameMsg::Lobby_Create;
			msg << this->m_localPID;
			m_client.Send(msg);

		}

		static uint32_t lobbyID = 0;
		ImGui::InputInt("LobbyID", (int*)&lobbyID);
		ImGui::SameLine();

		if (ImGui::Button("Join"))
		{
			JoinLobby(lobbyID);
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

	if (m_client.IsConnected())
	{
		m_client.Update();
	}
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

		LOG_INFO("Ping: %fs", std::chrono::duration<double>(timeNow - this->m_timeThen).count());
		break;
	}
	case GameMsg::Server_AssignID:
	{
		msg >> this->m_localPID;

		LOG_INFO("YOUR ID IS: %lu", this->m_localPID);
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
			m_players.insert(std::make_pair(remotePlayerID, m_demoScene->CreatePlayerEntity()));
		}

		message<GameMsg> message;
		message.header.id = GameMsg::Lobby_Accepted;

		break;
	}
	case GameMsg::Lobby_Accepted:
	{
		msg >> m_gameID;
		LOG_INFO("You are now in lobby: %lu", m_gameID);
		break;
	}
	case GameMsg::Game_Update:
	{
		uint32_t playerID;
		msg >> playerID;
		if (m_players.find(playerID) != m_players.end())
		{
			comp::Transform t;
			msg >> t;
			*m_players.at(playerID).GetComponent<comp::Transform>() = t;
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
	LOG_INFO("Joining Lobby %d ...", lobbyID);
	message<GameMsg> msg;

	msg.header.id = GameMsg::Lobby_Join;
	msg << this->m_localPID << lobbyID;
	m_client.Send(msg);
}



void Game::OnShutdown()
{

}
