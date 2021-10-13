#pragma once
#include "net_common.h"
#include "net_message.h"
#include "RModel.h"

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
			uint32_t playerID;
		};

		struct Renderable
		{
			std::shared_ptr<RModel> model;
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
	};

	sm::Matrix GetMatrix(const component::Transform& transform);
	sm::Vector3 GetForward(const component::Transform& transform);
	sm::Vector3 GetUp(const component::Transform& transform);
};

namespace comp = ecs::component;