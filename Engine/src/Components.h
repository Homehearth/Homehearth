#pragma once
#include "RMesh.h"
#include <entt.hpp>

namespace ecs
{
	namespace component {
		struct Transform
		{
			sm::Vector3 position;
			sm::Vector3 rotation;
			sm::Vector3 scale;

			dx::ConstantBuffer<cbuffer::PerObject>* pConstantBuffer;
		};

		struct Mesh
		{
			RMesh* mesh;
		};
	}

	void OnTransformConstruct(entt::registry& reg, entt::entity entity);

	sm::Matrix GetMatrix(component::Transform& transform);
	sm::Vector3 GetForward(component::Transform& transform);
	sm::Vector3 GetUp(component::Transform& transform);

}

namespace comp = ecs::component;
