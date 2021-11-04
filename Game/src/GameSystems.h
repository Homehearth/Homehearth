#pragma once
#include "EnginePCH.h"
#include "Intersections.h"
#include "Game.h"

namespace GameSystems
{
	void MRayIntersectBoxSystem(Scene& scene);
	void RenderIsCollidingSystem(Scene& scene);
	void UpdateHealthbar(Scene& scene);
	void UpdateNamePlate(Scene& scene);
}

