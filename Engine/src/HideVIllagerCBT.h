#pragma once
#include "ActionNodeBT.h"

namespace BT
{
	class HideVillagerCBT final : public BT::ActionNode
	{
	public:
		HideVillagerCBT(const std::string& name, Entity entity, Blackboard* blackboard);
		~HideVillagerCBT() override = default;
		BT::NodeStatus Tick() override;
	private:
		Entity entity;
		Blackboard* blackboard;
	};

}