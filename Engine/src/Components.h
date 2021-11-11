#pragma once
#include "net_message.h"
#include "RModel.h"
#include "RAnimator.h"
#include "ResourceManager.h"

namespace ecs
{
	enum Component : uint32_t
	{
		TRANSFORM,
		VELOCITY,
		ANIMATOR_NAME,
		MESH_NAME,
		NAME_PLATE,
		HEALTH,
		BOUNDING_ORIENTED_BOX,
		BOUNDING_SPHERE,
		LIGHT,
		PLAYER,
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
			sm::Quaternion rotation;
			sm::Vector3 scale = sm::Vector3(1);
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

		struct Animator
		{
			std::shared_ptr<RAnimator> animator;
		};

		// Used on server side
		struct AnimatorName 
		{
			std::string name = "";
		};
	
		struct MeshName 
		{
			std::string name = "";
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
				if (obb != nullptr)
				{
					model = ResourceManager::Get().GetResource<RModel>("Cube.obj");
				}
				else if (sphere != nullptr)
				{
					model = ResourceManager::Get().GetResource<RModel>("Sphere.obj");
				}
			}
		};
		

		struct TemporaryPhysics
		{
			struct Force
			{
				sm::Vector3 force = sm::Vector3(5, 0, 0);
				float drag = 5.f;
				bool isImpulse = true;
				bool wasApplied = false;
				float actingTime = 1.0f;
			};

			std::vector<Force> forces;
		};

		struct Velocity
		{
			sm::Vector3 vel;
			sm::Vector3 oldVel;
		};

		struct Player
		{
			enum class State
			{
				IDLE,
				ATTACK,
				TURN,
				DEAD
			} state = State::IDLE;

			float runSpeed;
			sm::Vector3 targetForward;
			sm::Vector3 spawnPoint;
			float respawnTimer;
			bool isReady = false;
		};

	

		struct NPC
		{
			enum class State
			{
				ASTAR,
				IDLE,
				CHASE
			} state;
			float movementSpeed = 15.f;
			float attackRange = 10.f;
			bool hostile;
			uint32_t currentNodeTarget = static_cast<uint32_t>(-1);
			std::vector<Node*> path;
			Node* currentNode;
			Entity currentClosest;
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

		struct IAbility
		{
			float cooldown = 1.5f;
			float cooldownTimer = 0.f;
			
			float delay = 0.1f;
			float delayTimer = 0.f;

			float lifetime = 5.f;

			bool isReady = false;
			bool isUsing = false;

			Ray_t targetRay;
		};

		struct CombatStats : public IAbility
		{
			float attackDamage = 5.f;
			float attackRange = 10.0f;
			bool isRanged = false;
			float projectileSpeed = 10.f;

			sm::Vector3 targetDir;
		};

		struct SelfDestruct
		{
			float lifeTime;
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
	sm::Vector3 GetRight(const component::Transform& transform);
	bool StepRotateTo(sm::Quaternion& rotation, const sm::Vector3& targetVector, float t);
	bool StepTranslateTo(sm::Vector3& translation, const sm::Vector3& target, float t);
	
	bool Use(component::IAbility* abilityComponent);

	component::TemporaryPhysics::Force GetGravityForce();

};

namespace comp = ecs::component;