#include "EnginePCH.h"
#include "VillagerTargetNodeCBT.h"

BT::VillagerTargetNodeCBT::VillagerTargetNodeCBT(const std::string& name, Entity entity)
	:ActionNode(name),
	entity(entity),
	refreshRate(5.0f)
{
	this->NewIdlePosTimer.Start();
}

BT::VillagerTargetNodeCBT::~VillagerTargetNodeCBT()
{
}

BT::NodeStatus BT::VillagerTargetNodeCBT::Tick()
{
	Cycle* cycle = Blackboard::Get().GetValue<Cycle>("cycle");
	sm::Vector3 targetPosition;

	if(cycle == nullptr)
	{
		LOG_WARNING("Failed to get cycle...");
		return BT::NodeStatus::FAILURE;
	}

	//focus to get to home node;
	if(*cycle == Cycle::NIGHT)
	{
		comp::Villager* villager = entity.GetComponent<comp::Villager>();
		if(villager && villager->homeNode)
		{
			targetPosition = villager->homeNode->position;
			Blackboard::Get().AddValue<sm::Vector3>("villagerTarget" + std::to_string(entity), targetPosition);
			return BT::NodeStatus::SUCCESS;
		}
	}
	//focus to get to the well
	else
	{
		comp::Villager* villager = entity.GetComponent<comp::Villager>();
		if(NewIdlePosTimer.GetElapsedTime<std::chrono::seconds>() > refreshRate)
		{
			NewIdlePosTimer.Start();
			targetPosition =  villager->idlePos.at(rand() % villager->idlePos.size());
			Blackboard::Get().AddValue<sm::Vector3>("villagerTarget" + std::to_string(entity), targetPosition);
		}
		return BT::NodeStatus::SUCCESS;
	}


	return BT::NodeStatus::SUCCESS;

}
