#pragma once
#include "EnginePCH.h"
#include "Game.h"

class Game;

namespace GameSystems
{
	void MRayIntersectBoxSystem(Scene& scene);
	void RenderIsCollidingSystem(Scene& scene);
	void UpdateHealthbar(Scene& scene);
	void CheckLOS(Game* game);
	void UpdateMainPlayer(Game* game);
}