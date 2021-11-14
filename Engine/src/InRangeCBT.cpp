#include "EnginePCH.h"
#include "InRangeCBT.h"

BT::InRangeCBT::InRangeCBT(const std::string& name, Entity entity)
	:ConditionNode(name),
	entity(entity)
{
}

BT::NodeStatus BT::InRangeCBT::Tick()
{
	comp::Transform* transform = entity.GetComponent<comp::Transform>();
	comp::NPC* npc = entity.GetComponent<comp::NPC>();
	Entity* target = Blackboard::Get().GetValue<Entity>("target" + std::to_string(entity));

	if(transform == nullptr || npc == nullptr)
	{
		LOG_ERROR("Failed to get components from entity");
		return BT::NodeStatus::FAILURE;
	}
	if(target == nullptr)
	{
		LOG_ERROR("Failed to get target position");
		return BT::NodeStatus::FAILURE;
	}

	if (sm::Vector3::Distance(transform->position, target->GetComponent<comp::Transform>()->position) <= npc->attackRange)
	{
		return BT::NodeStatus::SUCCESS;
	}
	else
	{
		return BT::NodeStatus::FAILURE;
	}
}
