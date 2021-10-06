#include "Game.h"
#include "RMesh.h"
#include "DemoScene.h"

#include "Components.h"

Game::Game()
	: Engine()
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
				m_client.PingServer();
			}
			if (ImGui::Button("Host")) 
			{

			}
			if (ImGui::Button("Join"))
			{

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
		while (!m_client.m_messagesIn.empty())
		{
			auto msg = m_client.m_messagesIn.pop_front();
			switch (msg.header.id)
			{
			case GameMsg::Server_AssignID:
			{
				msg >> this->localPID;

				LOG_INFO("YOUR ID IS: %lu", this->localPID);
				break;
			}
			}
		}
	}

}

void Game::OnShutdown()
{
	
}
