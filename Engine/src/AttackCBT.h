#pragma once
#include "ActionNodeBT.h"
namespace BT
{
	class AttackCBT : public BT::ActionNode
	{
	public:
		AttackCBT(const std::string& name, Entity entity, Blackboard* blackboard);
		~AttackCBT() override = default;

		//Executes the attack ability against the current target
		BT::NodeStatus Tick() override;

	private:
		Entity entity;
		Blackboard* blackboard;
	};
}


