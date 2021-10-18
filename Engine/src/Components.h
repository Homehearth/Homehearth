#pragma once
#include "net_common.h"
#include "net_message.h"
#include "RModel.h"
#include "RDebugMesh.h"
#include "ResourceManager.h"

namespace ecs
{
	namespace component {

		//Collider components
		using DirectX::BoundingOrientedBox;
		using DirectX::BoundingSphere;
		
		struct Transform
		{
			sm::Vector3 position;
			sm::Vector3 rotation;
			sm::Vector3 scale = sm::Vector3(1);

			friend network::message<network::GameMsg>& operator<<(network::message<network::GameMsg>& msg, const ecs::component::Transform& data)
			{
				msg << data.position.x << data.position.y << data.position.z;
				msg << data.rotation.x << data.rotation.y << data.rotation.z;
				msg << data.scale.x << data.scale.y << data.scale.z;
				return msg;
			}

			friend network::message<network::GameMsg>& operator >> (network::message<network::GameMsg>& msg, ecs::component::Transform& data)
			{
				msg >> data.scale.z >> data.scale.y >> data.scale.x;
				msg >> data.rotation.z >> data.rotation.y >> data.rotation.x;
				msg >> data.position.z >> data.position.y >> data.position.x;
				return msg;
			}
		};

		struct Network
		{
			uint32_t id;
		};

		struct Renderable
		{
			std::shared_ptr<RModel>		model;
			basic_model_matrix_t		data;
		};
		
		struct RenderableDebug
		{
			std::shared_ptr<RModel> 	model;
			basic_model_matrix_t 		data;
			collider_hit_t				isColliding;
			void InitRenderable(entt::registry& reg, const entt::entity curr)
			{
				BoundingOrientedBox* obb = reg.try_get<BoundingOrientedBox>(curr);
				BoundingSphere* sphere = reg.try_get<BoundingSphere>(curr);
				if(obb != nullptr)
				{
					model = ResourceManager::Get().GetResource<RModel>("cube.obj");
				}
				else if(sphere != nullptr)
				{
					model = ResourceManager::Get().GetResource<RModel>("Sphere.obj");
				}
			}
		};
		
		struct Velocity
		{
			sm::Vector3 vel;
		};

		struct Player
		{
			float runSpeed;
		};

		template<uint8_t ID>
		struct Tag
		{
			uint8_t id = ID;
		};

	};

	sm::Matrix GetMatrix(const component::Transform& transform);
	sm::Vector3 GetForward(const component::Transform& transform);
	sm::Vector3 GetUp(const component::Transform& transform);
};

namespace comp = ecs::component;