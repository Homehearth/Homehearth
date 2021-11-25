#pragma once
#include "EnginePCH.h"
#include "Game.h"

class Game;

namespace GameSystems
{
	void MRayIntersectBoxSystem(Scene& scene);
	void RenderIsCollidingSystem(Scene& scene);
	void UpdateHealthbar(Game* game);
	void CheckLOS(Game* game);
	void UpdatePlayerVisuals(Game* game);
	void DisplayUpgradeDefences(Game* game);
}