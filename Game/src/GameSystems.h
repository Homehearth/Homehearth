#pragma once
#include "EnginePCH.h"

namespace GameSystems
{
	void MRayIntersectBoxSystem(Scene& scene);
	void RenderIsCollidingSystem(Scene& scene);
	void UpdateHealthbar(Scene& scene);
	void CheckLOS(const sm::Vector3& camPos, const sm::Vector3& playerPos, const std::vector<dx::BoundingSphere>& mapColliders);
}