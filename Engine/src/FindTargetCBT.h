#pragma once
#include "ActionNodeBT.h"

namespace BT
{
	class FindTargetCBT final : public BT::ActionNode
	{
	public:
			FindTargetCBT(const std::string& name, Entity entity);
			~FindTargetCBT() override;
			BT::NodeStatus Tick() override;
	private:
		Entity entity;
	};
}

