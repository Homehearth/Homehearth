#pragma once

#include "ActionNodeBT.h"
class VillagerTargetNodeCBT final : public BT::ActionNode
{
public:
	VillagerTargetNodeCBT(const std::string& name, Entity entity);
	~VillagerTargetNodeCBT() override;
	BT::NodeStatus Tick() override;
private:
	Entity entity;
};

