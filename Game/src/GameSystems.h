#pragma once
#include "Game.h"
#include "Intersections.h"
#include <Client.h>
namespace GameSystems
{
	void UserInputSystem(Scene& scene, Client& client);
	void MRayIntersectBoxSystem(Scene& scene);
}
