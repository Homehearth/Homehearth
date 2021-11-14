#pragma once
#include "Timer.h"
namespace BT
{
	
	class GenPathCBT final : public BT::ActionNode
	{
	public:
		GenPathCBT(const std::string& name, Entity entity);
		~GenPathCBT() override = default;
		BT::NodeStatus Tick() override;
	private:
		Entity entity;
		Timer generatePathTimer;
		float refreshRate;
	};
}


