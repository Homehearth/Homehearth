#pragma once
#include "BehaviorTreeBT.h"


namespace AIBehaviors
{
	void UpdateBlackBoard(HeadlessScene& scene);

	//This functions can define different behaviors trees.
	
	//Simple ai that searches for the nearest playerand moves towards it
	std::shared_ptr<BT::FallbackNode> GetFocusPlayerAIBehavior(Entity entity);
	//Ai that will focus on buildings instead of the players (may pick a player as a target if close enough...)
	std::shared_ptr<BT::FallbackNode> GetFocusBuildingAIBehavior(Entity entity);
}
