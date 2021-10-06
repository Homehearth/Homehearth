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
			std::shared_ptr<RMesh> mesh;
			 basic_model_matrix_t data;
		};

		struct Velocity
		{
			sm::Vector3 vel;
		};

		struct Player
		{
			float runSpeed;
		};

		struct BoxCollider
		{
			sm::Vector3 center;
			sm::Vector3 norm[3];
			float halfSize[3];
		};
		struct SphereCollider
		{
			sm::Vector3 center;
			float radius;
		};
	};

	sm::Matrix GetMatrix(const component::Transform& transform);
	sm::Vector3 GetForward(const component::Transform& transform);
	sm::Vector3 GetUp(const component::Transform& transform);

};

namespace comp = ecs::component;

