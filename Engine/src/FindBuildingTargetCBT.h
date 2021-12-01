#pragma once

namespace BT
{
	class FindBuildingTargetCBT final : public BT::ActionNode
	{
	public:
		FindBuildingTargetCBT(const std::string& name, Entity entity);
		~FindBuildingTargetCBT() override = default;
		BT::NodeStatus Tick() override;
	private:
		Entity entity;
		Timer changeTargetTimer;
		float refreshRate;
		float aggroRange;
	};
}