#pragma once
#include "ActionNodeBT.h"

namespace BT
{

	class FleeEnemyCBT final : public BT::ActionNode
	{
	public:
		FleeEnemyCBT(const std::string& name, Entity entity);
		~FleeEnemyCBT() override = default;
		BT::NodeStatus Tick() override;
	private:
		Entity entity;
	};
}