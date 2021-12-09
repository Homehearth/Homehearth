#pragma once
#include "ActionNodeBT.h"
namespace BT
{
	class TeleportNodeCBT final : public BT::ActionNode
	{
	public:
		TeleportNodeCBT(const std::string& name, Entity entity, Blackboard* blackboard);
		~TeleportNodeCBT() override;
		BT::NodeStatus Tick() override;
	private:
		Entity entity;
		Blackboard* blackboard;
	};
}