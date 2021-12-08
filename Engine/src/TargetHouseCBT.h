#pragma once
#include "ActionNodeBT.h"
namespace BT
{
	class TargetHouseCBT : public BT::ActionNode
	{
	public:
		TargetHouseCBT(const std::string& name, Entity entity, Blackboard* blackboard, float aggroRange = 25.f);
		~TargetHouseCBT() override = default;

		//attempt to target the nearest house
		BT::NodeStatus Tick() override;

	private:
		Entity entity;

		//Range which AI can target house
		float aggroRange;
		Blackboard* blackboard;
	};
}
