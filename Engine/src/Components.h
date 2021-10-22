#pragma once
#include "net_message.h"
#include "RModel.h"
#include "RDebugMesh.h"
#include "ResourceManager.h"

namespace ecs
{
	enum Component : uint32_t
	{
		NETWORK,
		TRANSFORM,
		MESH_NAME,
		BOUNDING_ORIENTED_BOX,
		BOUNDING_SPHERE,
		COMPONENT_COUNT,
		COMPONENT_MAX = 32
	};

	namespace component {

		//Collider components
		using DirectX::BoundingOrientedBox;
		using DirectX::BoundingSphere;
		
		struct Transform
		{
			sm::Vector3 position;
			sm::Vector3 rotation;
			sm::Vector3 scale = sm::Vector3(1);

			friend network::message<GameMsg>& operator<<(network::message<GameMsg>& msg, const ecs::component::Transform& data)
			{
				msg << data.position << data.rotation << data.scale;
				return msg;
			}

			friend network::message<GameMsg>& operator >> (network::message<GameMsg>& msg, ecs::component::Transform& data)
			{
				msg >> data.scale >> data.rotation >> data.position;
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

		// Used on server side
		struct MeshName 
		{
			std::string name;
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
		struct NPC
		{
			float movementSpeed;
			float attackRange;
			bool hostile;
		};
		struct Light
		{
			light_t lightData;
			int index;
		};

		template<uint8_t ID>
		struct Tag
		{
			uint8_t id = ID;
		};
		struct PotentialField
		{
			float chargeAmount;
			bool positive;
		};

	};

	sm::Matrix GetMatrix(const component::Transform& transform);
	sm::Vector3 GetForward(const component::Transform& transform);
	sm::Vector3 GetUp(const component::Transform& transform);
};

network::message<GameMsg>& operator<<(network::message<GameMsg>& msg, const sm::Vector3& data);

network::message<GameMsg>& operator>>(network::message<GameMsg>& msg, sm::Vector3& data);

network::message<GameMsg>& operator<<(network::message<GameMsg>& msg, const sm::Vector4& data);

network::message<GameMsg>& operator>>(network::message<GameMsg>& msg, sm::Vector4& data);


namespace comp = ecs::component;