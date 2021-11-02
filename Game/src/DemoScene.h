#pragma once
#include "EnginePCH.h"
#include "GameSystems.h"
#include "Lights.h"
#include "Game.h"

#include "Tags.h"

namespace sceneHelp
{
	Entity CreatePlayerEntity(HeadlessScene& scene, uint32_t playerID);
	Entity CreateLightEntity(Scene& scene, sm::Vector4 pos, sm::Vector4 dir, sm::Vector4 col, float range, TypeLight type, UINT enabled);

	void CreateMainMenuScene(Game* game);
	void CreateLobbyScene(Game* game);
	void CreateGameScene(Game* engine);
	void CreateJoinLobbyScene(Game* game);
	void CreateLoadingScene(Game* game);

	void SetupMainMenuScreen(Game* game);
	void SetupLobbyJoinScreen(Game* game);
	void SetupInLobbyScreen(Game* game);
	void SetupInGameScreen(Scene& scene);
	void SetupOptionsScreen(Scene& scene);
	void SetupLoadingScene(Game* game);
}
