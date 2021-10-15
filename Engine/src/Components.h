#pragma once
#include "net_message.h"
#include "RModel.h"
#include "RDebugMesh.h"

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

network::message<GameMsg>& operator<<(network::message<GameMsg>& msg, const sm::Vector3& data);

network::message<GameMsg>& operator >> (network::message<GameMsg>& msg, sm::Vector3& data);

namespace comp = ecs::component;