#pragma once
#include "EnginePCH.h"
#include "GameSystems.h"
#include "Engine.h"

class DemoScene : public SceneBuilder<DemoScene>
{
private:
	Engine* m_engine;

	Entity m_player;
	Entity m_chest;

	Camera m_gameCamera;
	Camera m_debugCamera;
	sm::Vector3 m_oldGameCameraPosition;
	sm::Vector3 m_oldDebugCameraPosition;
	uint32_t* m_playerID, *m_gameID;
	Client& m_client;

public:
	DemoScene(Engine& engine, Client& client, uint32_t* playerID, uint32_t* gameID);
	
	Entity CreatePlayerEntity();

	//Camera
	void SetUpCamera();
	void CameraUpdate(float deltaTime);
};