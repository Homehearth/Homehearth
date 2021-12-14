#pragma once
#include "net_message.h"
#include "RModel.h"
#include "RAnimator.h"
#include "ResourceManager.h"
#include "BehaviorTreeBT.h"
#include <stack>
#include "ModelIdentifier.h"

namespace ecs
{
	enum Component : uint32_t
	{
		PARTICLEMITTER,
		TRANSFORM,
		VELOCITY,
		MESH_NAME,
		ANIMATOR_NAME,
		HEALTH,
		BOUNDING_ORIENTED_BOX,
		BOUNDING_SPHERE,
		PLAYER,
		COST,

		// Kills and Deaths
		KD,
		COMPONENT_COUNT,
		COMPONENT_MAX = 32
	};

	namespace component 
	{
		struct OrientedBoxCollider : dx::BoundingOrientedBox
		{
			// Empty for now
		};

		struct SphereCollider : dx::BoundingSphere
		{
			std::set<Entity> list;
		};

		struct BoxCollider : dx::BoundingBox
		{
			// Empty for now
		};

		struct Transform
		{
			sm::Vector3 position;
			sm::Quaternion rotation;
			sm::Vector3 scale = sm::Vector3(1);

			bool syncColliderScale = false;
		};

		struct Cost
		{
			uint32_t cost;
		};

		struct Decal
		{
			sm::Matrix viewPoint;
			// Life span in seconds.
			float lifespan = 10.0f;

			Decal(const Transform& t)
			{
				// Be positioned slightly above.
				sm::Vector3 position = t.position;
				position.y = 10.0f;
				position.x += 0.0001f;
				position.z -= 0.0001f;

				sm::Vector3 lookAt = t.position;
				lookAt.y = 0;
				viewPoint = dx::XMMatrixLookAtLH(position, lookAt, { 0.0f, 1.0f, 0.0f });
			}
		};

		struct Watermill
		{
			float theta = 0;
		};

		struct EmitterParticle
		{
			sm::Vector3							positionOffset	= { 0,0,0 };
			UINT								nrOfParticles	= 0;
			ParticleMode						type			= ParticleMode::BLOOD;
			float								lifeTime		= 0.f;
			float								sizeMulitplier	= 0.f;
			float								speed			= 0.f;
			bool								hasDeathTimer	= false;
			float								lifeLived		= 0.f;

			std::string textureName								= "";
			std::string opacityTextureName						= "";

			std::shared_ptr<RTexture>			texture			= nullptr;
			std::shared_ptr<RTexture>			opacityTexture	= nullptr;
			ComPtr<ID3D11Buffer>				particleBuffer	= nullptr;
			ComPtr<ID3D11ShaderResourceView>	particleSRV		= nullptr;
			ComPtr<ID3D11UnorderedAccessView>	particleUAV		= nullptr;

			component::Transform				transformCopy;

			EmitterParticle(sm::Vector3 positionOffset = {0,0,0}, int nrOfParticles = 10, float sizeMulitplier = 1.f, ParticleMode type = ParticleMode::BLOOD, float lifeTime = 2.f, float speed = 1, bool hasDeathTimer = false)
			{
				textureName = "thisisfine.png";
				opacityTextureName = "round_Opacity.png";

				if (type == ParticleMode::BLOOD)
				{
					textureName = "Blood.png";
				}
				else if (type == ParticleMode::LEAF)
				{
				}
				else if (type == ParticleMode::WATERSPLASH)
				{
					textureName = "waterSplash.png";
				}
				else if (type == ParticleMode::SMOKEPOINT || type == ParticleMode::SMOKEAREA)
				{
					textureName = "smoke.png";
					opacityTextureName = "smoke_Opacity.png";
				}
				else if (type == ParticleMode::SPARKLES)
				{
				}
				else if (type == ParticleMode::RAIN)
				{
					textureName = "drop.png";
					opacityTextureName = "drop_Opacity.png";
				}
				else if (type == ParticleMode::DUST)
				{
				}				
				else if (type == ParticleMode::MAGEHEAL)
				{
					textureName = "MageHeal.png";
				}
				else if (type == ParticleMode::MAGERANGE || type == ParticleMode::EXPLOSION)
				{
					textureName = "fire.png";
					opacityTextureName = "fire_Opacity.png";
				}

				texture = ResourceManager::Get().GetResource<RTexture>(textureName);
				opacityTexture = ResourceManager::Get().GetResource<RTexture>(opacityTextureName);

				if (!texture)
				{
					LOG_ERROR("Couldnt load particle texture %s", textureName.c_str());
				}
				if (!opacityTexture)
				{
					LOG_ERROR("Couldnt load particle opacity texture %s", opacityTextureName.c_str());
				}
				
				this->nrOfParticles		= (UINT)nrOfParticles;
				this->type				= type;
				this->lifeTime			= lifeTime;
				this->sizeMulitplier	= sizeMulitplier;
				this->speed				= speed;
				this->positionOffset	= positionOffset;
				this->hasDeathTimer		= hasDeathTimer;
			}
		};

		struct ParticleEmitter 
		{
			sm::Vector3		positionOffset	= { 0,0,0 };
			UINT			nrOfParticles	= 0;
			ParticleMode	type			= ParticleMode::BLOOD;
			float			lifeTime		= 0.f;
			float			sizeMulitplier	= 0.f;
			float			speed			= 0.f;
			bool			hasDeathTimer	= false;
			float			lifeLived		= 0.f;

			ParticleEmitter(sm::Vector3 positionOffset = { 0,0,0 }, int nrOfParticles = 10, float sizeMulitplier = 1.f, ParticleMode type = ParticleMode::BLOOD, float lifeTime = 2.f, float speed = 1, bool hasDeathTimer = false)
			{
				this->nrOfParticles		= (UINT)nrOfParticles;
				this->type				= type;
				this->lifeTime			= lifeTime;
				this->sizeMulitplier	= sizeMulitplier;
				this->speed				= speed;
				this->positionOffset	= positionOffset;
				this->hasDeathTimer		= hasDeathTimer;
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
			bool						castShadow = true;
		};

		struct Animator
		{
			std::shared_ptr<RAnimator>  animator;
			bool						updating = true;
		};

		struct AnimationState
		{
			EAnimationType lastSend = EAnimationType::NONE;	//Send to user last time
			EAnimationType toSend	= EAnimationType::IDLE;	//Going to be send this update
		};

		struct AudioState
		{
			std::queue<audio_t> data;
		};

		// Used on server side
		struct AnimatorName
		{
			AnimName name;
		};

		struct MeshName
		{
			NameType name;
		};

		struct RenderableDebug
		{
			std::shared_ptr<RModel> 	model;
			basic_model_matrix_t 		data;
			collider_hit_t				isColliding;
			void InitRenderable(entt::registry& reg, const entt::entity curr)
			{
				OrientedBoxCollider* obb = reg.try_get<OrientedBoxCollider>(curr);
				SphereCollider* sphere = reg.try_get<SphereCollider>(curr);
				BoxCollider* box = reg.try_get<BoxCollider>(curr);
				if (obb != nullptr)
				{
					model = ResourceManager::Get().GetResource<RModel>("Cube.obj");
				}
				else if (sphere != nullptr)
				{
					model = ResourceManager::Get().GetResource<RModel>("Sphere.obj");
				}
				else if (box != nullptr)
				{
					model = ResourceManager::Get().GetResource<RModel>("Cube.obj");
				}
			}
		};

		struct BehaviorTree
		{
			std::shared_ptr<BT::ParentNode> root;
		};

		struct Villager
		{
			std::vector<Node*> path;
			//Stress implementation - fix later -
			std::vector<sm::Vector3> idlePos = {sm::Vector3(250.f, 0.f, -320.f),
												sm::Vector3(212.f, 0.f, -297.f),
												sm::Vector3(237.f, 0.f, -297.f),
												sm::Vector3(325.f, 0.f, -370.f),
												sm::Vector3(330.f, 0.f, -285.f),
												sm::Vector3(135.f, 0.f, -374.f)}; //Positions villager can go and idle at
			Node* currentNode;
			Entity homeHouse;
			float movementSpeed = 15.f;
			bool isHiding = false;
			bool isFleeing = false;
		};

		struct House
		{
			NameType houseType = NameType::EMPTY;
			NameType doorType = NameType::EMPTY;
			NameType roofType = NameType::EMPTY;
			Entity door;
			Entity houseRoof;
			Node* homeNode = nullptr; //AI can walk to this node to attack this house
			bool isDead;
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

		struct BezierAnimation
		{
			float speed = 1.0f;
			bool loop = false;
			std::vector<sm::Vector3> translationPoints;
			std::vector<sm::Vector3> scalePoints;
			std::vector<sm::Quaternion> rotationPoints;
			float time = 0.0f;

			std::function<void()> onFinish;
		};

		struct Velocity
		{
			sm::Vector3 vel;
			sm::Vector3 oldVel;
		};

		struct Player
		{
			enum class PlayerType : uint16_t
			{
				NONE,
				PLAYER_ONE,
				PLAYER_TWO,
				PLAYER_THREE,
				PLAYER_FOUR
			} playerType = PlayerType::NONE;
			enum class State
			{
				IDLE,
				LOOK_TO_MOUSE,
				WALK
			} state = State::IDLE;

			enum class Class
			{
				WARRIOR,
				MAGE
			} classType = Class::WARRIOR;

			entt::meta_type primaryAbilty;
			entt::meta_type secondaryAbilty;
			entt::meta_type moveAbilty;

			float runSpeed;

			InputState inputState;
			InputState lastInputState;

			sm::Vector3 mousePoint;
			sm::Vector3 fowardDir;

			sm::Vector3 spawnPoint;
			float respawnTimer;
			bool isReady = false;
			bool reachable = true;
			bool wantsToSkipDay = false;

			char name[13] = {};

			ShopItem		shopItem = ShopItem::None;

			//Place defence option 
			float			buildDistance = 32.0f;		//A tiles width is ~8
			bool			rotateDefence = false;
		};

		struct KillDeaths
		{
			unsigned int kills = 0;
			unsigned int deaths = 0;
		};

		struct PlayerReference
		{
			Entity player;
		};

		
		struct TileSet
		{
			std::vector<std::pair<UINT, UINT>> coordinates;
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
			bool hostile;
			std::vector<Node*> path;
			Node* currentNode;
			Entity currentClosest;
		};

		struct Light
		{
			light_t lightData;
			int index;
			float flickerTimer = 1.f;
			float maxFlickerTime = 1.f;
			bool increase;
			float enabledTimer = 1.f;
		};

		struct Health
		{
			float maxHealth = 100.f;
			float currentHealth = 100.f;
			bool isAlive = true;
			int upgradeLevel = 0;
		};

		struct IAbility
		{
			// set this for cooldown
			float cooldown = 1.5f;
			// !DO NOT TOUCH!
			float cooldownTimer = 0.f;
			bool isCooldownActive = true;

			// set this for delay before ability is actually used after the cooldown is done and the ecs::UseAbility has bee called
			float delay = 0.1f;
			// !DO NOT TOUCH!
			float delayTimer = 0.f;

			// the time it takes to use
			float useTime = 1.0f;
			// !DO NOT TOUCH!
			float useTimer = 0.f;

			// alter movement speed during use
			// == 1 -> normal, < 1 -> slow, > 1 -> fast
			float movementSpeedAlt = 0.2f;
			// lifetime of the ability, for instance lifetime of any created collider
			float lifetime = 5.f;

			// !DO NOT TOUCH!
			bool isReady = false;
			// !DO NOT TOUCH!
			bool isUsing = false;

			// set to be target for ability
			sm::Vector3 targetPoint;

			int upgradeLevel = 0;
		};

		//---------- WARRIOR ABILITIES ----------
		struct MeleeAttackAbility : public IAbility
		{
			//Just to keep it not empty for now
			float attackDamage = 5.f;
			float attackRange = 10.0f;
		};

		struct HeroLeapAbility : public IAbility
		{
			float damage = 10.f;
			float damageRadius = 20.f;
			float maxRange = 30.f;
		};

		struct ShieldBlockAbility : public IAbility
		{
			Entity shieldCollider;

			float maxDamage = 50.f;
			float damageTaken = 0.0f;

			float timeSinceUse = 0.0f;
		};

		struct DashAbility : public IAbility
		{
			//The duration for the force
			float duration = 1.0f;
			//amount of force applied when used
			float force = 25.0f;
			//Store velocity that was before dash was applied
			sm::Vector3 velocityBeforeDash = { 0.0f,0.0f,0.0f };
		};
		//------------------END----------------------

		//---------- MAGE ABILITIES ----------
		struct RangeAttackAbility : public IAbility
		{
			float attackDamage = 5.f;
			float attackRange = 10.0f;
			float projectileSpeed = 10.f;
			float projectileSize = 1.0f;
		};

		struct HealAbility : public IAbility
		{
			float healAmount = 40.f;
			float range = 50.f;
		};
		struct BlinkAbility : public IAbility
		{
			//The distance ability teleports Entity forward 
			float distance = 50.0f;
		};
		//------------------END----------------------

		//----------------- EFFECTS -----------------
		struct Stun
		{
			float stunTime = 5.0f;
			float movementSpeedAlt = 0.0f;
		};

		struct Invincible {
			float time = 1.0f;
		};

		//------------------END----------------------


		struct SelfDestruct
		{
			float lifeTime;
			std::function<void()> onDestruct;
		};


		struct ITag
		{
			TagType id;
		};

		template<TagType ID>
		struct Tag : ITag
		{
			Tag() {
				id = ID;
			}
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

		using namespace entt::literals;
		if constexpr (std::is_base_of_v<component::IAbility, T>)
		{
			entt::meta<T>().type()
				.base<component::IAbility>()
				.func<&Entity::GetComponentRef<T>, entt::as_ref_t>("get"_hs)
				.func<&Entity::HasComponent<T>>("has"_hs);

			LOG_INFO("Registered ability %s", entt::resolve<T>().info().name().data());
		}
		else if constexpr (std::is_base_of_v<component::ITag, T>) {
			entt::meta<T>().type()
				.base<component::ITag>()
				.func<&Entity::GetComponentRef<T>, entt::as_ref_t>("get"_hs)
				.func<&Entity::HasComponent<T>>("has"_hs);

			LOG_INFO("Registered tag %s", entt::resolve<T>().info().name().data());
		}
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


	void CancelAbility(component::IAbility* abilityComponent);
	void CancelAbility(Entity entity, entt::meta_type abilityType);


	bool IsPlayerUsingAnyAbility(Entity player);

	component::IAbility* GetAbility(Entity entity, entt::meta_type abilityType);

	component::TemporaryPhysics::Force GetGravityForce();

};

namespace comp = ecs::component;