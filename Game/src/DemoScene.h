#pragma once
#include "EnginePCH.h"
#include "GameSystems.h"
#include "Engine.h"

class DemoScene : public SceneBuilder<DemoScene, Scene>
{
private:
	Engine* m_engine;

public:
	DemoScene(Engine& engine);
	
	Entity CreatePlayerEntity(uint32_t playerID);

	//Camera
	void SetUpCamera();
	void CheckIfSwappedCamera();
	void InitializeGameCam();
	Camera m_gameCamera;
	Camera m_debugCamera;
};