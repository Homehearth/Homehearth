#pragma once
#include "EnginePCH.h"
#include "GameSystems.h"
#include "Engine.h"

#include "Tags.h"

class DemoScene : public SceneBuilder<DemoScene, Scene>
{
private:

public:
	DemoScene(Engine& engine);
	
	Entity CreatePlayerEntity(uint32_t playerID);
};

namespace sceneHelp
{
	Entity CreatePlayerEntity(HeadlessScene& scene, uint32_t playerID);
	
	void CreateMainMenuScene(Engine& engine);
	void CreateLobbyScene(Engine& engine);
	void CreateGameScene(Engine& engine);

}
