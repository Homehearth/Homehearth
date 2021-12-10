#include "EnginePCH.h"
#include "AttackCBT.h"

BT::AttackCBT::AttackCBT(const std::string& name, Entity entity, Blackboard* blackboard)
	:ActionNode(name),
	entity(entity),
	blackboard(blackboard)
{
}

BT::NodeStatus BT::AttackCBT::Tick()
{
	//get the ability enemy have
	comp::MeleeAttackAbility* attackMAbility = entity.GetComponent<comp::MeleeAttackAbility>();
	comp::RangeAttackAbility* attackRAbility = entity.GetComponent<comp::RangeAttackAbility>();
	comp::IAbility* attackAbility = nullptr;

	//Store ability in variable
	if (attackMAbility)
		attackAbility = attackMAbility;
	else if (attackRAbility)
		attackAbility = attackRAbility;

	Entity* target = blackboard->GetValue<Entity>("target" + std::to_string(entity));

	//Catch nullptr
	if(attackAbility == nullptr || target == nullptr)
	{
		LOG_ERROR("Failed to get correct data from entity");
		return BT::NodeStatus::FAILURE;
	}

	//Check if target is a house (houses get their position from OBB)
	if(target->GetComponent<comp::House>())
		attackAbility->targetPoint = target->GetComponent<comp::OrientedBoxCollider>()->Center;
	else
		attackAbility->targetPoint = target->GetComponent<comp::Transform>()->position;

	attackAbility->targetPoint.y = 0.0f;

	//Use the ability stored in attackAbility
	if (ecs::UseAbility(attackAbility))
	{
		comp::AnimationState* animState = entity.GetComponent<comp::AnimationState>();
		//Activate attack animation
		if (animState)
			animState->toSend = EAnimationType::PRIMARY_ATTACK;

		entity.UpdateNetwork();
	};

	return BT::NodeStatus::SUCCESS;
}
