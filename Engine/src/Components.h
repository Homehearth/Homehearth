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

		struct Decal
		{
			RTexture* decal = nullptr;
			sm::Matrix viewPoint;
			// Life span in seconds.
			float lifespan = 5.0f;

			Decal(const Transform& t, RTexture* decal = nullptr)
			{
				this->decal = decal;

				// Be positioned slightly above.
				sm::Vector3 position = t.position;
				position.y += 10.0f;
				position.x += 0.0001f;
				position.z -= 0.0001f;

				sm::Vector3 lookAt = t.position;
				lookAt.y = 0;
				viewPoint = dx::XMMatrixLookAtLH(position, lookAt, { 0.0f, 1.0f, 0.0f });
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
			bool						isSolid = true;
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

			enum class Class
			{
				WARRIOR,
				MAGE
			} classType = Class::WARRIOR;

			entt::meta_type primaryAbilty;
			entt::meta_type secondaryAbilty;

			float runSpeed;

			sm::Vector3 mousePoint;
			sm::Vector3 fowardDir;

			sm::Vector3 spawnPoint;
			float respawnTimer;
			bool isReady = false;
		};

		struct Node
		{
			float f = FLT_MAX, g = FLT_MAX, h = FLT_MAX;
			sm::Vector3 position;
			Vector2I id;
			std::vector<Node*> connections;
			ecs::component::Node* parent;
			bool reachable = true;
			void ResetFGH()
			{
				f = FLT_MAX, g = FLT_MAX, h = FLT_MAX;
			}
			bool ConnectionAlreadyExists(Node* other)
			{
				for (Node* node : connections)
				{
					if (node == other)
					{
						return true;
					}
				}
				return false;
			}
		};

		struct NPC
		{
			enum class State
			{
				IDLE,
				ASTAR,
				CHASE
			} state;
			float movementSpeed = 15.f;
			float attackRange = 10.f;
			bool hostile;
			uint32_t currentNodeTarget = static_cast<uint32_t>(-1);
			std::vector<ecs::component::Node*> path;
			ecs::component::Node* currentNode;
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
			// set this for cooldown
			float cooldown = 1.5f;
			// !DO NOT TOUCH!
			float cooldownTimer = 0.f;
			
			// set this for delay before ability is actually used after the cooldown is done and the ecs::UseAbility has bee called
			float delay = 0.1f;
			// !DO NOT TOUCH!
			float delayTimer = 0.f;

			// the time it takes to use
			float useTime = 1.0f;
			// !DO NOT TOUCH!
			float useTimer = 0.f;

			// lifetime of the ability, for instance lifetime of any created collider
			float lifetime = 5.f;

			// !DO NOT TOUCH!
			bool isReady = false;
			// !DO NOT TOUCH!
			bool isUsing = false;

			// set to be target for ability
			sm::Vector3 targetPoint;
		};

		struct AttackAbility : public IAbility
		{
			float attackDamage = 5.f;
			float attackRange = 10.0f;
			bool isRanged = false;
			float projectileSpeed = 10.f;
		};

		struct HealAbility : public IAbility
		{
			float healAmount = 40.f;
			float range = 50.f;
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
	
	template<typename T>
	void RegisterAsAbility()
	{
		static_assert(std::is_base_of_v<component::IAbility, T> && "Component has to derive from comp::IAbility");

		using namespace entt::literals;
		entt::meta<T>().type()
			.base<component::IAbility>()
			.func<&Entity::GetComponentRef<T>, entt::as_ref_t>("get"_hs);
	}

	/**
	*	Warning: This function will modify the component
	*	If this ability is ready to be used, meaning isReady == true.
	*	If returns true the ability is set to being used, so it only returns true once with a interval of cooldown.
	*/
	bool UseAbility(component::IAbility* abilityComponent, sm::Vector3* targetPoint = nullptr);
	/**
	*	Warning: This function will modify the component
	*	If this ability is ready to be used, meaning isReady == true.
	*	If returns true the ability is set to being used, so it only returns true once with a interval of cooldown.
	*/
	bool UseAbility(Entity entity, entt::meta_type abilityType, sm::Vector3* targetPoint = nullptr);

	/**
	*	Warning: This function will modify the component
	*	Check if this ability is ready to used, meaning isUsing == true and delayTimer <= 0.
	*	If returns true the ability is reset, so it only returns true once after ecs::UseAbility is called on this component.
	*/
	bool ReadyToUse(component::IAbility* abilityComponent, sm::Vector3* targetPoint = nullptr);
	/**
	*	Warning: This function will modify the component
	*	Check if this ability is ready to used, meaning isUsing == true and delayTimer <= 0.
	*	If returns true the ability is reset, so it only returns true once after ecs::UseAbility is called on this component.
	*/
	bool ReadyToUse(Entity entity, entt::meta_type abilityType, sm::Vector3* targetPoint = nullptr);

	// returns if the ability is currently used
	bool IsUsing(const component::IAbility* abilityComponent);

	// returns if the ability is currently used
	bool IsUsing(Entity entity, entt::meta_type abilityType);

	bool IsPlayerUsingAnyAbility(Entity player);

	component::TemporaryPhysics::Force GetGravityForce();

};

namespace comp = ecs::component;