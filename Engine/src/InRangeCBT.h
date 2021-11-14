#pragma once
#include "ConditionNodeBT.h"
namespace BT
{
	class InRangeCBT : public BT::ConditionNode
	{
	public:
		InRangeCBT(const std::string& name, Entity entity);
		~InRangeCBT() override = default;
		//Checks if the target is within attack range
		BT::NodeStatus Tick() override;
	private:
		Entity entity;
	};
}

