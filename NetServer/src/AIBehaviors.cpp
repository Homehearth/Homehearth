#include "NetServerPCH.h"
#include "AIBehaviors.h"

void AIBehaviors::UpdateBlackBoard(HeadlessScene& scene)
{
	PlayersPosition_t players;
	scene.ForEachComponent<comp::Player, comp::Transform>([&](Entity entity, comp::Player& player, comp::Transform& transform)
	{
			players.positions.emplace_back(transform.position);
	});
	Blackboard::Get().AddValue("playersPosition", players);
}

BT::FallbackNode* AIBehaviors::GetSimpleAIBehavior(Entity entity)
{

		BT::FallbackNode* root = new BT::FallbackNode("root");

		BT::SequenceNode* seq1 = new BT::SequenceNode("seq1");

		BT::ActionNode* findTarget = new BT::FindTargetCBT("Find target", entity);
		BT::ActionNode* moveToTarget = new BT::MoveCBT("Move To Target", entity);

		root->AddChild(seq1);
		seq1->AddChild(findTarget);
		seq1->AddChild(moveToTarget);

		return root;
}
