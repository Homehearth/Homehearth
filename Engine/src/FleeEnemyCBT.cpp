#include "EnginePCH.h"
#include "FleeEnemyCBT.h"

BT::FleeEnemyCBT::FleeEnemyCBT(const std::string& name, Entity entity)
	:ActionNode(name),
	entity(entity)
{
}

BT::NodeStatus BT::FleeEnemyCBT::Tick()
{
	return BT::NodeStatus::SUCCESS;
}
