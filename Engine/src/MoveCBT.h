#pragma once
#include "ActionNodeBT.h"

namespace BT
{
	class MoveCBT final : public BT::ActionNode
	{
	public:
		MoveCBT(const std::string& name, Entity entity);
		~MoveCBT() override = default;
		BT::NodeStatus Tick() override;
	private:
		Entity entity;
	};
}
