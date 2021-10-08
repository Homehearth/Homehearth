#pragma once
#include "EnginePCH.h"
#include "Intersections.h"

namespace GameSystems
{
	void UserInputSystem(Scene& scene, Client& client);
	void MRayIntersectBoxSystem(Scene& scene);

	//***Remove later***
	void ChangeMaterialSystem(Scene& scene);
}
