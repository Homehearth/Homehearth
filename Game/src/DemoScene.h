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

public:
	DemoScene(Engine& engine);
	
	Entity CreatePlayerEntity(uint32_t playerID);
	Entity CreateLightEntity(sm::Vector4 pos, sm::Vector4 dir, sm::Vector4 col, float range, TypeLight type, UINT enabled);
};

namespace sceneHelp
{
	Entity CreatePlayerEntity(HeadlessScene& scene, uint32_t playerID);
	Entity CreateLightEntity(Scene& scene, sm::Vector4 pos, sm::Vector4 dir, sm::Vector4 col, float range, TypeLight type, UINT enabled);

	void CreateMainMenuScene(Engine& engine);
	void CreateLobbyScene(Engine& engine);
	void CreateGameScene(Engine& engine);

}
