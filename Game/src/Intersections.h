#pragma once
#include "Game.h"

namespace Intersect
{
	bool MouseRayIntersectBox(comp::BoxCollider& boxCollider, float& t);
	bool MouseRayIntersectSphere(comp::SphereCollider& sphereCollider, float& t);
}

