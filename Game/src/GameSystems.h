#pragma once
#include "EnginePCH.h"
#include "Intersections.h"

namespace GameSystems
{
	void MRayIntersectBoxSystem(Scene& scene);
	void RenderIsCollidingSystem(Scene& scene);
	void UpdateHealthbar(Scene& scene);
	void UpdatePlayerVisuals(Scene& scene);
	void CheckLOS(Scene& scene, const sm::Vector3& playerPos, const std::vector<dx::BoundingSphere>& mapColliders);
}

