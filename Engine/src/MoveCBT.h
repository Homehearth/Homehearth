#pragma once
#include "ActionNodeBT.h"
#include "Components.h"
#include "Timer.h"


namespace BT
{
	class MoveCBT final : public BT::ActionNode
	{
	public:

		//Construct
		MoveCBT(const std::string& name, Entity entity);
		~MoveCBT() override = default;

		//The main function called on when the tree is ticking
		BT::NodeStatus Tick() override;
	private:
		//will try to get out of a node that is unrechable,
	    //Can happen in edge cases when AI takes a shortcut
	    //over nodes and happens to retrieve that it is closest to a non-rechable node.
		static bool EscapeCurrentNode(Entity entity);

		Entity entity;

		//Timer for when to try escape again
		Timer timerEscape;
		float refreshRateOnEscape;
	};
}
