#include "Game.h"
#include "RMesh.h"
#include "DemoScene.h"

#include "Components.h"

using namespace std::placeholders;

Game::Game()
	: m_client(std::bind(&Game::CheckIncoming, this, _1))
	, Engine()
{
	this->localPID = 0;

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
	Scene& demo = DemoScene(*this, m_client).GetScene();

	//Set as current scene
	SetScene(demo);

	return true;
}

void Game::OnUserUpdate(float deltaTime)
{

	
	IMGUI(
	ImGui::Begin("Test");

	if (m_client.IsConnected())
	{
		if (ImGui::Button("Ping"))
		{
			PingServer();
		}
		if (ImGui::Button("Host"))
		{
			message<GameMsg> msg;

			msg.header.id = GameMsg::Client_CreateLobby;
			msg << this->localPID;
			m_client.Send(msg);

			LOG_INFO("Creating game lobby!");

		}


		
		static uint32_t lobbyID = 0;
		ImGui::InputInt("LobbyID", (int*)&lobbyID);
		ImGui::SameLine();

		if (ImGui::Button("Join"))
		{
			message<GameMsg> msg;

			msg.header.id = GameMsg::Client_JoinLobby;
			msg << this->localPID << lobbyID;
			m_client.Send(msg);
		}
	}
	else {
		static char buffer[64];
		strcpy(buffer, "127.0.0.1");
		ImGui::InputText("IP", buffer, 64);
		if (ImGui::Button("Connect"))
		{
			m_client.Connect(buffer, 4950);
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

		LOG_INFO("Ping: %fs", std::chrono::duration<double>(timeNow - this->timeThen).count());
		break;
	}
	case GameMsg::Server_AssignID:
	{
		msg >> this->localPID;

		LOG_INFO("YOUR ID IS: %lu", this->localPID);
		break;
	}
	case GameMsg::Game_AddPlayer:
	{
		uint32_t remotePlayerID;
		msg >> remotePlayerID;
		LOG_INFO("Player with ID: %ld has joined the game!", remotePlayerID);
		break;
	}
	}
}

void Game::PingServer()
{
	message<GameMsg> msg = {};
	msg.header.id = GameMsg::Server_GetPing;
	msg << this->localPID;

	this->timeThen = std::chrono::system_clock::now();
	m_client.Send(msg);
}



void Game::OnShutdown()
{
	
}
