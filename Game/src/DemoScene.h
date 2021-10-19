#pragma once
#include "EnginePCH.h"
#include "GameSystems.h"
#include "Engine.h"
#include "Lights.h"

#include "Tags.h"

class DemoScene : public SceneBuilder<DemoScene, Scene>
{
private:
	Entity m_directionalLight;
	Entity m_pointLight;

	Entity m_player;
	Entity m_chest;

	Camera m_gameCamera;
	Camera m_debugCamera;
	sm::Vector3 m_oldGameCameraPosition;
	sm::Vector3 m_oldDebugCameraPosition;
	uint32_t* m_playerID, *m_gameID;
	Client& m_client;
public:
	DemoScene(Engine& engine);
	
	Entity CreatePlayerEntity(uint32_t playerID);
	Entity CreateLightEntity(sm::Vector4 pos, sm::Vector4 dir, sm::Vector4 col, float range, TypeLight type, UINT enabled);
};