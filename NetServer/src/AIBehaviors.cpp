#include "NetServerPCH.h"
#include "AIBehaviors.h"

void AIBehaviors::UpdateBlackBoard(HeadlessScene& scene)
{
	PlayersPosition_t players;
	scene.ForEachComponent<comp::Player, comp::Transform>([&](Entity entity, comp::Player& player, comp::Transform& transform)
	{
			players.players.emplace_back(entity);
	});
	Blackboard::Get().AddValue("players", players);
}

BT::FallbackNode* AIBehaviors::GetSimpleAIBehavior(Entity entity)
{
	 auto root = new BT::FallbackNode("root");

	 auto fallback1 = new BT::FallbackNode("fallback1");
	 auto fallback2 = new BT::FallbackNode("fallback2");
	 auto seq1 = new BT::SequenceNode("seq1");
	 auto seq2 = new BT::SequenceNode("seq2");
	 auto seq3 = new BT::SequenceNode("seq3");
		


	auto findTarget = new BT::FindTargetCBT("FindTarget", entity);
	auto moveToTarget = new BT::MoveCBT("MoveToTarget", entity);
	auto attackTarget = new BT::AttackCBT("AttackTarget", entity);
	auto inRange = new BT::InRangeCBT("InRange", entity);
	auto genPath = new BT::GenPathCBT("GenPath", entity);

	root->AddChild(fallback1);
		fallback1->AddChild(seq1);
			seq1->AddChild(findTarget);
			seq1->AddChild(fallback2);
				fallback2->AddChild(seq2);
				fallback2->AddChild(seq3);
					seq2->AddChild(inRange);
					seq2->AddChild(attackTarget);
					seq3->AddChild(genPath);
					seq3->AddChild(moveToTarget);

	return root;
}
