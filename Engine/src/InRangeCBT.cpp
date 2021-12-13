#include "EnginePCH.h"
#include "InRangeCBT.h"

BT::InRangeCBT::InRangeCBT(const std::string& name, Entity entity, Blackboard* blackboard)
	:ConditionNode(name),
	entity(entity),
	blackboard(blackboard)
{
}

BT::NodeStatus BT::InRangeCBT::Tick()
{
	sm::Vector3 position;

	comp::Transform* transform = entity.GetComponent<comp::Transform>();

	comp::MeleeAttackAbility* attackMAbility = entity.GetComponent<comp::MeleeAttackAbility>();
	comp::RangeAttackAbility* attackRAbility = entity.GetComponent<comp::RangeAttackAbility>();
	Entity* target = blackboard->GetValue<Entity>("target" + std::to_string(entity));

	if (transform == nullptr || attackMAbility && attackRAbility)
	{
		LOG_ERROR("Failed to get components from entity");
		return BT::NodeStatus::FAILURE;
	}
	if (target == nullptr)
	{
		LOG_ERROR("Failed to get target position");
		return BT::NodeStatus::FAILURE;
	}
	comp::House* house = target->GetComponent<comp::House>();
	comp::Transform* targetTransform = target->GetComponent<comp::Transform>();
	if (targetTransform == nullptr)
	{
		LOG_ERROR("Failed to get target position");
		return BT::NodeStatus::FAILURE;
	}

	float extendedRange = 2.f;
	if (target->GetComponent<comp::Tag<DEFENCE>>())
	{
		extendedRange = 2.5f;
	}

	//If target is a house
	if (house && house->homeNode)
	{
		position = house->homeNode->position;
	}
	else
	{
		position = target->GetComponent<comp::Transform>()->position;
	}

	float distance = sm::Vector3::Distance(transform->position, position);
	if (attackMAbility)
	{
		if (distance <= (attackMAbility->attackRange * extendedRange))
		{
			return BT::NodeStatus::SUCCESS;
		}
	}
	else if (attackRAbility)
	{
		if (distance <= attackRAbility->attackRange)
		{
			return BT::NodeStatus::SUCCESS;
		}
	}

	return BT::NodeStatus::FAILURE;
}
