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
		VELOCITY,
		MESH_NAME,
		NAME_PLATE,
		BOUNDING_ORIENTED_BOX,
		BOUNDING_SPHERE,
		LIGHT,
		PLAYER,
		TILE,
		COMPONENT_COUNT,
		COMPONENT_MAX = 32
	};

	namespace component {

		//Collider components
		using DirectX::BoundingOrientedBox;
		using DirectX::BoundingSphere;
		
		struct Transform
		{
			sm::Vector3 previousPosition;
			sm::Vector3 position;
			sm::Quaternion rotation;
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
			uint32_t id = UINT32_MAX;
		};

		struct Renderable
		{
			std::shared_ptr<RModel>		model;
			basic_model_matrix_t		data;
			bool						visible = true;
		};

		// Used on server side
		struct MeshName 
		{
			std::string name;
		};

		struct NamePlate
		{
			std::string namePlate;
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
			enum class State
			{
				IDLE,
				ATTACK,
				TURN
			} state = State::IDLE;

			float runSpeed;
			sm::Vector3 targetForward;
			bool isReady = false;
		};

		struct Enemy
		{
			float temp = 0;
		};

		struct Light
		{
			light_t lightData;
			int index;
		};

		struct Health
		{
			float maxHealth = 100.f;
			float currentHealth = 100.f;
			bool isAlive = true;
		};

		struct CombatStats
		{
			float attackSpeed = 1.f;
			float attackDamage = 5.f;
			float attackLifeTime = 5.f;
			bool isRanged = false;
			float projectileSpeed = 10.f;

			bool isAttacking = false;
			float cooldownTimer = 0.f;
			Ray_t targetRay;
			sm::Vector3 targetDir;
		};

		struct Attack
		{
			float lifeTime;
			float damage;
		};


		template<uint8_t ID>
		struct Tag
		{
			uint8_t id = ID;
		};

		struct Tile 
		{
			TileType type;
			sm::Vector2 gridID;
			float halfWidth;
		};

	};

	sm::Matrix GetMatrix(const component::Transform& transform);
	sm::Vector3 GetForward(const component::Transform& transform);
	sm::Vector3 GetRight(const component::Transform& transform);
	bool StepRotateTo(sm::Quaternion& rotation, const sm::Vector3& targetVector, float t);
	bool StepTranslateTo(sm::Vector3& translation, const sm::Vector3& target, float t);

};

namespace comp = ecs::component;