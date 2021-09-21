#pragma once
#include "RMesh.h"

namespace ecs
{
	namespace component {
		struct Transform
		{
			sm::Vector3 position;
			sm::Vector3 rotation;
			sm::Vector3 scale;

			std::unique_ptr<dx::ConstantBuffer<sm::Matrix>> pConstantBuffer;
		};

		struct Renderable
		{
			RMesh* mesh;
			std::unique_ptr<dx::ConstantBuffer<sm::Matrix>> pConstantBuffer;
		};
	};

	void OnTransformConstruct(entt::registry& reg, entt::entity entity);
	void OnTransformUpdate(entt::registry& reg, entt::entity entity);

	void OnRenderableConstruct(entt::registry& reg, entt::entity entity);


	sm::Matrix GetMatrix(component::Transform& transform);
	sm::Vector3 GetForward(component::Transform& transform);
	sm::Vector3 GetUp(component::Transform& transform);

};

namespace comp = ecs::component;

