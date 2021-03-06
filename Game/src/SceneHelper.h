#pragma once
#include "Lights.h"
#include "EnginePCH.h"

// Forward declaration to not get circular include
class Game;

namespace sceneHelp
{
	Entity CreateLightEntity(Scene& scene, sm::Vector4 pos, sm::Vector4 dir, sm::Vector4 col, float range, float intensity, TypeLight type, UINT enabled);

	void CreateMainMenuScene(Game* game);
	void CreateLobbyScene(Game* game);
	void CreateGameScene(Game* engine);
	void CreateJoinLobbyScene(Game* game);
	void CreateLoadingScene(Game* game);
	void CreateOptionsScene(Game* game);
	void CreateGameOverScene(Game* game);

	void SetupMainMenuScreen(Game* game);
	void SetupLobbyJoinScreen(Game* game);
	void SetupInLobbyScreen(Game* game);
	void SetupInGameScreen(Game* game);
	void SetupOptionsScreen(Game* game);
	void SetupLoadingScene(Game* game);
	void SetupGameOverScreen(Game* game);

	bool LoadMapColliders(Game* game);
	void LoadGameScene(Game* game);
	void LoadResources(Game* game);
}
