#pragma once
#include "BehaviorTreeBT.h"


namespace AIBehaviors
{
	void UpdateBlackBoard(HeadlessScene& scene, Blackboard* blackboard);
	void ClearBlackBoard(HeadlessScene& scene, Blackboard* blackboard);

	//This functions can define different behaviors trees.
	
	//Simple ai that searches for the nearest playerand moves towards it
	std::shared_ptr<BT::FallbackNode> GetFocusPlayerAIBehavior(Entity entity, Blackboard* blackboard);
	//Ai that will focus on buildings instead of the players (may pick a player as a target if close enough...)
	std::shared_ptr<BT::FallbackNode> GetFocusBuildingAIBehavior(Entity entity, Blackboard* blackboard);
	//Simple ai that handles villager logic
	std::shared_ptr<BT::FallbackNode> GetVillagerAIBehavior(Entity entity, Blackboard* blackboard);
}
