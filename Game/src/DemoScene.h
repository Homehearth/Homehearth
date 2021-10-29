#pragma once
#include "EnginePCH.h"
#include "GameSystems.h"
#include "Engine.h"
#include "Lights.h"

#include "Tags.h"

namespace sceneHelp
{
	Entity CreatePlayerEntity(HeadlessScene& scene, uint32_t playerID);
	Entity CreateLightEntity(Scene& scene, sm::Vector4 pos, sm::Vector4 dir, sm::Vector4 col, float range, TypeLight type, UINT enabled);

	void CreateMainMenuScene(Engine& engine);
	void CreateLobbyScene(Engine& engine);
	void CreateGameScene(Engine& engine);
	void CreateConnectScene(Engine& engine, Client* c);
	void CreateJoinLobbyScene(Engine& engine);

	void SetupMainMenuScreen(Engine& engine, Scene& scene);
	void SetupLobbyJoinScreen(Scene& scene, Window* pWindow);
	void SetupInLobbyScreen(Scene& scene);
	void SetupInGameScreen(Scene& scene);
	void SetupOptionsScreen(Scene& scene);
	void SetupConnectScreen(Engine& e, Client* c);
}
