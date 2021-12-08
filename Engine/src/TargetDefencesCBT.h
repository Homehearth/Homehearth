#pragma once
#include "ActionNodeBT.h"
namespace BT
{
	class TargetDefencesCBT : public BT::ActionNode
	{
	public:
		TargetDefencesCBT(const std::string& name, Entity entity, float aggroRange = 25.f);
		~TargetDefencesCBT() override = default;

		//Executes the attack ability against the current target
		BT::NodeStatus Tick() override;

	private:
		Entity entity;
		float aggroRange;
	};
}