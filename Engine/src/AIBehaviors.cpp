#include "EnginePCH.h"
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

std::shared_ptr<BT::FallbackNode> AIBehaviors::GetFocusPlayerAIBehavior(Entity entity)
{
	 std::shared_ptr<BT::FallbackNode> root = std::make_shared<BT::FallbackNode>(BT::FallbackNode("root"));

	 auto fallback1 = std::make_shared<BT::FallbackNode>(BT::FallbackNode("fallback1"));
	 auto fallback2 = std::make_shared<BT::FallbackNode>(BT::FallbackNode("fallback2"));
	 auto fallback3 = std::make_shared<BT::FallbackNode>(BT::FallbackNode("fallback3"));
	 auto seq1 = std::make_shared<BT::SequenceNode>(BT::SequenceNode("seq1"));
	 auto seq2 = std::make_shared<BT::SequenceNode>(BT::SequenceNode("seq2"));
	 auto seq3 = std::make_shared<BT::SequenceNode>(BT::SequenceNode("seq3"));
		


	auto findTarget = std::make_shared<BT::FindTargetCBT>(BT::FindTargetCBT("FindTarget", entity));
	auto findBuildingTarget = std::make_shared<BT::FindBuildingTargetCBT>(BT::FindBuildingTargetCBT("FindBuildingTarget", entity));
	auto moveToTarget = std::make_shared<BT::MoveCBT>(BT::MoveCBT("MoveToTarget", entity));
	auto attackTarget = std::make_shared<BT::AttackCBT>(BT::AttackCBT("AttackTarget", entity));
	auto inRange = std::make_shared<BT::InRangeCBT>(BT::InRangeCBT("InRange", entity));
	auto genPath = std::make_shared<BT::GenPathCBT>(BT::GenPathCBT("GenPath", entity));

	root->AddChild(fallback1);
		fallback1->AddChild(seq1);
			seq1->AddChild(fallback3);
			seq1->AddChild(fallback2);
				fallback3->AddChild(findTarget);
				fallback3->AddChild(findBuildingTarget);
				fallback2->AddChild(seq2);
				fallback2->AddChild(seq3);
					seq2->AddChild(inRange);
					seq2->AddChild(attackTarget);
					seq3->AddChild(genPath);
					seq3->AddChild(moveToTarget);
			

	return root;
}

std::shared_ptr<BT::FallbackNode> AIBehaviors::GetFocusBuildingAIBehavior(Entity entity)
{
	std::shared_ptr<BT::FallbackNode> root = std::make_shared<BT::FallbackNode>(BT::FallbackNode("root"));

	auto fallback1 = std::make_shared<BT::FallbackNode>(BT::FallbackNode("fallback1"));
	auto fallback2 = std::make_shared<BT::FallbackNode>(BT::FallbackNode("fallback2"));
	auto fallback3 = std::make_shared<BT::FallbackNode>(BT::FallbackNode("fallback3"));
	auto seq1 = std::make_shared<BT::SequenceNode>(BT::SequenceNode("seq1"));
	auto seq2 = std::make_shared<BT::SequenceNode>(BT::SequenceNode("seq2"));
	auto seq3 = std::make_shared<BT::SequenceNode>(BT::SequenceNode("seq3"));



	auto findBuildingTarget = std::make_shared<BT::FindBuildingTargetCBT>(BT::FindBuildingTargetCBT("FindBuildingTarget", entity));
	auto findTarget = std::make_shared<BT::FindTargetCBT>(BT::FindTargetCBT("FindTarget", entity));
	auto moveToTarget = std::make_shared<BT::MoveCBT>(BT::MoveCBT("MoveToTarget", entity));
	auto attackTarget = std::make_shared<BT::AttackCBT>(BT::AttackCBT("AttackTarget", entity));
	auto inRange = std::make_shared<BT::InRangeCBT>(BT::InRangeCBT("InRange", entity));
	auto genPath = std::make_shared<BT::GenPathCBT>(BT::GenPathCBT("GenPath", entity));

	root->AddChild(fallback1);
	fallback1->AddChild(seq1);
	seq1->AddChild(fallback3);
	seq1->AddChild(fallback2);
	fallback3->AddChild(findBuildingTarget);
	fallback3->AddChild(findTarget);
	fallback2->AddChild(seq2);
	fallback2->AddChild(seq3);
	seq2->AddChild(inRange);
	seq2->AddChild(attackTarget);
	seq3->AddChild(genPath);
	seq3->AddChild(moveToTarget);


	return root;

}
