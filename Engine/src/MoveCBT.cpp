#include "EnginePCH.h"
#include "MoveCBT.h"

BT::MoveCBT::MoveCBT(const std::string& name, Entity entity)
	:ActionNode(name),
	entity(entity)
{
}


BT::NodeStatus BT::MoveCBT::Tick()
{
	//Temp function to get the direction to target and add that to velocity
	sm::Vector3* targetPosition = Blackboard::Get().GetValue<sm::Vector3>("target"+std::to_string(entity));
	comp::Transform* transform = entity.GetComponent<comp::Transform>();

	if(targetPosition == nullptr)
	{
		LOG_ERROR("Target position returned as nullptr...");
		return BT::NodeStatus::FAILURE;
	}

	if(transform == nullptr)
	{
		LOG_ERROR("Transform component returned as nullptr...");
		return BT::NodeStatus::FAILURE;
	}

	sm::Vector3 direction = *targetPosition - transform->position;
	direction.Normalize();

	entity.GetComponent<comp::Velocity>()->vel = direction * entity.GetComponent<comp::NPC>()->movementSpeed;

	return BT::NodeStatus::SUCCESS;
}
