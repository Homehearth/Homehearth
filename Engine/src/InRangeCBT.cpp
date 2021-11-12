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
	sm::Vector3* targetPos = Blackboard::Get().GetValue<sm::Vector3>("target" + std::to_string(entity));

	if(transform == nullptr || npc == nullptr)
	{
		LOG_ERROR("Failed to get components from entity");
		return BT::NodeStatus::FAILURE;
	}
	if(targetPos == nullptr)
	{
		LOG_ERROR("Failed to get target position");
		return BT::NodeStatus::FAILURE;
	}

	if (sm::Vector3::Distance(transform->position, *targetPos) <= npc->attackRange)
	{
		return BT::NodeStatus::SUCCESS;
	}
	else
	{
		return BT::NodeStatus::FAILURE;
	}
}
