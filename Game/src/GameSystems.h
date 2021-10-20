#pragma once
#include "EnginePCH.h"
#include "Intersections.h"

namespace GameSystems
{
	void UserInputSystem(Scene& scene, Client& client);
	void MRayIntersectBoxSystem(Scene& scene);
	void RenderIsCollidingSystem(Scene& scene);
}
