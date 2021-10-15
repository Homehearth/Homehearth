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

		struct Light
		{
			light_t lightData;
			int index;
		};
	};

	sm::Matrix GetMatrix(const component::Transform& transform);
	sm::Vector3 GetForward(const component::Transform& transform);
	sm::Vector3 GetUp(const component::Transform& transform);


};
// serialize
network::message<network::GameMsg>& operator << (network::message<network::GameMsg>& msg, const ecs::component::Transform& data);

network::message<network::GameMsg>& operator >> (network::message<network::GameMsg>& msg, ecs::component::Transform& data);

namespace comp = ecs::component;

