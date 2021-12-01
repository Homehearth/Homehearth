#include "EnginePCH.h"
#include "VillagerTargetNodeCBT.h"

VillagerTargetNodeCBT::VillagerTargetNodeCBT(const std::string& name, Entity entity)
	:ActionNode(name),
	entity(entity)
{
}

VillagerTargetNodeCBT::~VillagerTargetNodeCBT()
{
}

BT::NodeStatus VillagerTargetNodeCBT::Tick()
{
	Cycle* cycle = Blackboard::Get().GetValue<Cycle>("cycle");
	sm::Vector3 targetPosition;

	if(cycle == nullptr)
	{
		LOG_WARNING("Failed to get cycle...");
		return BT::NodeStatus::FAILURE;
	}

	if(*cycle == Cycle::MORNING)
	{
		//focus to get to the well
	}
	else if(*cycle == Cycle::NIGHT)
	{
		//focus to get to home node;
		comp::Villager* villager = entity.GetComponent<comp::Villager>();
		if(villager && villager->homeNode)
		{
			targetPosition = villager->homeNode->position;
			Blackboard::Get().AddValue<sm::Vector3>("villagerTarget" + std::to_string(entity), targetPosition);
		}
	}



	return BT::NodeStatus::SUCCESS;

}
