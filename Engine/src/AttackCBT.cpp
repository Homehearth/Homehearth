#include "EnginePCH.h"
#include "AttackCBT.h"

BT::AttackCBT::AttackCBT(const std::string& name, Entity entity)
	:ActionNode(name),
	entity(entity)
{
}

BT::NodeStatus BT::AttackCBT::Tick()
{
	comp::MeleeAttackAbility* attackMAbility = entity.GetComponent<comp::MeleeAttackAbility>();
	comp::RangeAttackAbility* attackRAbility = entity.GetComponent<comp::RangeAttackAbility>();
	comp::IAbility* attackAbility = nullptr;

	if (attackMAbility)
		attackAbility = attackMAbility;
	else if (attackRAbility)
		attackAbility = attackRAbility;


	comp::Transform* transform = entity.GetComponent<comp::Transform>();
	Entity* target = Blackboard::Get().GetValue<Entity>("target" + std::to_string(entity));

	if(attackAbility == nullptr || transform == nullptr)
	{
		LOG_ERROR("Failed to get components from entity");
		return BT::NodeStatus::FAILURE;
	}
	if(target == nullptr)
	{
		LOG_ERROR("Failed to get target position");
		return BT::NodeStatus::FAILURE;
	}

	attackAbility->targetPoint = target->GetComponent<comp::Transform>()->position;

	if (ecs::UseAbility(attackAbility))
	{
		// Enemy Attacked
		comp::AnimationState* animState = entity.GetComponent<comp::AnimationState>();
		if (animState)
			animState->toSend = EAnimationType::PRIMARY_ATTACK;
	};

	return BT::NodeStatus::SUCCESS;
}
