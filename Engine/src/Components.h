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
			Transform renderForm;
			//dx::ConstantBuffer<sm::Matrix> constantBuffer;
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
	};

	void OnRenderableConstruct(entt::registry& reg, entt::entity entity);


	sm::Matrix GetMatrix(component::Transform& transform);
	sm::Vector3 GetForward(component::Transform& transform);
	sm::Vector3 GetUp(component::Transform& transform);

};

namespace comp = ecs::component;

