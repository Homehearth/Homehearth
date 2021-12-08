#pragma once
#include "ActionNodeBT.h"

namespace BT
{
	class TargetPlayerCBT : public BT::ActionNode
	{
	public:
		TargetPlayerCBT(const std::string& name, Entity entity, Blackboard* blackboard, float aggroRange = 25.f);
		~TargetPlayerCBT() override = default;

		//attempt to find the nearest player and selecting it as the target
		BT::NodeStatus Tick() override;

	private:
		Entity entity;
		float aggroRange;
		Blackboard* blackboard;
	};
}
