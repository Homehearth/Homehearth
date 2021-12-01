#include "EnginePCH.h"
#include "InRangeCBT.h"

BT::InRangeCBT::InRangeCBT(const std::string& name, Entity entity)
	:ConditionNode(name),
	entity(entity)
{
}

BT::NodeStatus BT::InRangeCBT::Tick()
{
	sm::Vector3 position;

	comp::Transform* transform = entity.GetComponent<comp::Transform>();

	comp::MeleeAttackAbility* attackMAbility = entity.GetComponent<comp::MeleeAttackAbility>();
	comp::RangeAttackAbility* attackRAbility = entity.GetComponent<comp::RangeAttackAbility>();
	Entity* target = Blackboard::Get().GetValue<Entity>("target" + std::to_string(entity));

	if(transform == nullptr || attackMAbility && attackRAbility)
	{
		LOG_ERROR("Failed to get components from entity");
		return BT::NodeStatus::FAILURE;
	}
	if(target == nullptr)
	{
		LOG_ERROR("Failed to get target position");
		return BT::NodeStatus::FAILURE;
	}
	comp::House* house = target->GetComponent<comp::House>();
	comp::Transform* targetTransform = target->GetComponent<comp::Transform>();
	if(targetTransform == nullptr)
	{
		LOG_ERROR("Failed to get target position");
		return BT::NodeStatus::FAILURE;
	}
	//If target is a house
	if(house && house->attackNode)
	{
		position = house->attackNode->position;
	}
	else
	{
		position = target->GetComponent<comp::Transform>()->position;
	}

	if (attackMAbility)
	{
		if (sm::Vector3::Distance(transform->position, position) <= attackMAbility->attackRange)
		{
			return BT::NodeStatus::SUCCESS;
		}
	}
	else if(attackRAbility)
	{
		if (sm::Vector3::Distance(transform->position, position) <= attackRAbility->attackRange)
		{
			return BT::NodeStatus::SUCCESS;
		}
	}

	return BT::NodeStatus::FAILURE;
}
