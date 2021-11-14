#pragma once
#include "BehaviorTreeBT.h"


namespace AIBehaviors
{
	void UpdateBlackBoard(HeadlessScene& scene);

	//This functions can define different behaviors trees.

	//Simple ai that searches for the nearest playerand moves towards it
	BT::FallbackNode* GetSimpleAIBehavior(Entity entity);
}
