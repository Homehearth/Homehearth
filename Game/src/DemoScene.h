#pragma once
#include "EnginePCH.h"
#include "GameSystems.h"
#include "Engine.h"

class DemoScene : public SceneBuilder<DemoScene>
{
private:
	Engine* m_engine;

	Entity m_player;

	Camera m_gameCamera;
	Camera m_debugCamera;
	sm::Vector3 m_oldGameCameraPosition;
	sm::Vector3 m_oldDebugCameraPosition;
public:
	DemoScene(Engine& engine, Client& client);

	Entity CreatePlayerEntity();

	//Camera
	void SetUpCamera();
	void CameraUpdate(float deltaTime);
};