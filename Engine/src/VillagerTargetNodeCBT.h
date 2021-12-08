#pragma once

#include "ActionNodeBT.h"
namespace BT
{
	class VillagerTargetNodeCBT final : public BT::ActionNode
	{
	public:
		VillagerTargetNodeCBT(const std::string& name, Entity entity, Blackboard* blackboard);
		~VillagerTargetNodeCBT() override;
		BT::NodeStatus Tick() override;
	private:
		Entity entity;
		Timer NewIdlePosTimer;
		float refreshRate;
		Blackboard* blackboard;
	};

}

