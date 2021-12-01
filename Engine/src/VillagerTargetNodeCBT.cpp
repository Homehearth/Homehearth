#include "EnginePCH.h"
#include "VillagerTargetNodeCBT.h"

BT::VillagerTargetNodeCBT::VillagerTargetNodeCBT(const std::string& name, Entity entity)
	:ActionNode(name),
	entity(entity)
{
	this->NewIdlePosTimer.Start();
	refreshRate = (5.0f + static_cast<float>(rand() % 3));
	comp::Villager* villager = entity.GetComponent<comp::Villager>();

	sm::Vector3 targetPosition;
	targetPosition = villager->idlePos.at(rand() % villager->idlePos.size());
	Blackboard::Get().AddValue<sm::Vector3>("villagerTarget" + std::to_string(entity), targetPosition);
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

	comp::Villager* villager = entity.GetComponent<comp::Villager>();

	if (villager->isFleeing)
		refreshRate = 2.0f;

	//focus to get to home node;
	if(*cycle == Cycle::NIGHT && villager && !villager->homeHouse.IsNull())
	{
		villager->movementSpeed = 30.f;
		if(villager && !villager->homeHouse.IsNull())
		{
			comp::House* house = villager->homeHouse.GetComponent<comp::House>();
			if(house->attackNode->reachable)
			{
				targetPosition = house->attackNode->position;
				Blackboard::Get().AddValue<sm::Vector3>("villagerTarget" + std::to_string(entity), targetPosition);
				return BT::NodeStatus::SUCCESS;
			}
			else
				return BT::NodeStatus::FAILURE;
		}
		else if(villager == nullptr || villager->homeHouse.IsNull())
		{
			return BT::NodeStatus::FAILURE; //House may be destroyed
		}
	}
	//focus to idle around the village
	else
	{
		if(*cycle == Cycle::DAY)
			villager->movementSpeed = 15.f;
		else if (*cycle == Cycle::DAY)
			villager->movementSpeed = 30.f;

		comp::Velocity* velocity = entity.GetComponent<comp::Velocity>();
		if(NewIdlePosTimer.GetElapsedTime<std::chrono::seconds>() > refreshRate)
		{
			NewIdlePosTimer.Start();
			targetPosition =  villager->idlePos.at(rand() % villager->idlePos.size());
			Blackboard::Get().AddValue<sm::Vector3>("villagerTarget" + std::to_string(entity), targetPosition);
		}
		else if(velocity)
		{
			comp::AnimationState* animState = entity.GetComponent<comp::AnimationState>();
			if (animState)
			{
				if (velocity->vel.Length() < 0.01f)
					animState->toSend = EAnimationType::IDLE;
			}
		}

		return BT::NodeStatus::SUCCESS;
	}

	return BT::NodeStatus::SUCCESS;

}
