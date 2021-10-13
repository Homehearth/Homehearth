#pragma once
#include "EnginePCH.h"
#include "GameSystems.h"
#include "Engine.h"

class DemoScene : public SceneBuilder<DemoScene, Scene>
{
private:
	Engine* m_engine;

	Camera m_gameCamera;
	Camera m_debugCamera;
	sm::Vector3 m_oldGameCameraPosition;
	sm::Vector3 m_oldDebugCameraPosition;

public:
	DemoScene(Engine& engine);
	
	Entity CreatePlayerEntity();
	Entity m_player;

	//Camera
	void SetUpCamera();
	void CameraUpdate(float deltaTime);
};