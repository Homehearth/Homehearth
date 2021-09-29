#pragma once
#include "RMesh.h"

namespace ecs
{
	namespace component {
		
		struct Transform
		{
			sm::Vector3 position;
			sm::Vector3 rotation;
			sm::Vector3 scale = sm::Vector3(1);
		};

		struct Renderable
		{
			RMesh* mesh;
			Transform transformCopy;
		};

		struct Velocity
		{
			sm::Vector3 vel;
		};

		struct Player
		{
			float runSpeed;
		};
	};

	sm::Matrix GetMatrix(component::Transform& transform);
	sm::Vector3 GetForward(component::Transform& transform);
	sm::Vector3 GetUp(component::Transform& transform);

};

namespace comp = ecs::component;

