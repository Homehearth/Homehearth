#include "EnginePCH.h"
#include "AIBehaviors.h"

void AIBehaviors::UpdateBlackBoard(HeadlessScene& scene, Blackboard* blackboard)
{
	PlayersPosition_t* players = blackboard->GetValue<PlayersPosition_t>("players");

	if (!players)
	{
		PlayersPosition_t playersNew;

		scene.ForEachComponent<comp::Player, comp::Transform>([&](Entity entity, comp::Player& player, comp::Transform& transform)
			{
				playersNew.players.emplace_back(entity);
			});

		scene.ForEachComponent<comp::Villager, comp::Transform>([&](Entity entity, comp::Villager& player, comp::Transform& transform)
			{
				playersNew.players.emplace_back(entity);
			});

		blackboard->AddValue("players", playersNew);
	}
}

void AIBehaviors::ClearBlackBoard(HeadlessScene& scene, Blackboard* blackboard)
{
	PlayersPosition_t* players = blackboard->GetValue<PlayersPosition_t>("players");

	if (players)
	{
		players->players.clear();
		blackboard->ClearValue<PlayersPosition_t>("players");
	}
}

std::shared_ptr<BT::FallbackNode> AIBehaviors::GetFocusPlayerAIBehavior(Entity entity, Blackboard* blackboard)
{
	std::shared_ptr<BT::FallbackNode> root = std::make_shared<BT::FallbackNode>(BT::FallbackNode("root"));

	auto fallback1 = std::make_shared<BT::FallbackNode>(BT::FallbackNode("fallback1"));
	auto fallback2 = std::make_shared<BT::FallbackNode>(BT::FallbackNode("fallback2"));
	auto fallback3 = std::make_shared<BT::FallbackNode>(BT::FallbackNode("fallback3"));
	auto seq1 = std::make_shared<BT::SequenceNode>(BT::SequenceNode("seq1"));
	auto seq2 = std::make_shared<BT::SequenceNode>(BT::SequenceNode("seq2"));
	auto seq3 = std::make_shared<BT::SequenceNode>(BT::SequenceNode("seq3"));

	const float maxAggro = 600.f;
	const float closeRange = 30.f;
	//Try to find target that is close to AI
	auto findClosePlayerTarget = std::make_shared<BT::TargetPlayerCBT>(BT::TargetPlayerCBT("FindClosePlayerTarget", entity, blackboard, closeRange));
	auto findCloseBuildingTarget = std::make_shared<BT::TargetHouseCBT>(BT::TargetHouseCBT("FindCloseBuildingTarget", entity, blackboard, closeRange));
	auto findCloseDefenseTarget = std::make_shared<BT::TargetHouseCBT>(BT::TargetHouseCBT("FindCloseBuildingTarget", entity, blackboard, closeRange));

	//If nothing nearby was found, look across the map
	auto findPlayer = std::make_shared<BT::TargetPlayerCBT>(BT::TargetPlayerCBT("FindTarget", entity, blackboard, maxAggro));
	auto findBuildingTarget = std::make_shared<BT::TargetHouseCBT>(BT::TargetHouseCBT("FindBuildingTarget", entity, blackboard, maxAggro));
	auto findDefenseTarget = std::make_shared<BT::TargetHouseCBT>(BT::TargetHouseCBT("FindBuildingTarget", entity, blackboard, maxAggro));

	auto moveToTarget = std::make_shared<BT::MoveCBT>(BT::MoveCBT("MoveToTarget", entity, blackboard));
	auto attackTarget = std::make_shared<BT::AttackCBT>(BT::AttackCBT("AttackTarget", entity, blackboard));
	auto inRange = std::make_shared<BT::InRangeCBT>(BT::InRangeCBT("InRange", entity, blackboard));
	auto genPath = std::make_shared<BT::GenPathCBT>(BT::GenPathCBT("GenPath", entity, blackboard));

	root->AddChild(fallback1);
	fallback1->AddChild(seq1);
	seq1->AddChild(fallback3);
	seq1->AddChild(fallback2);

	//Find a target
	fallback3->AddChild(findClosePlayerTarget);
	fallback3->AddChild(findCloseBuildingTarget);
	fallback3->AddChild(findCloseDefenseTarget);
	fallback3->AddChild(findPlayer);
	fallback3->AddChild(findBuildingTarget);
	fallback3->AddChild(findDefenseTarget);

	fallback2->AddChild(seq2);
	fallback2->AddChild(seq3);
	seq2->AddChild(inRange);
	seq2->AddChild(attackTarget);
	seq3->AddChild(genPath);
	seq3->AddChild(moveToTarget);


	return root;
}

std::shared_ptr<BT::FallbackNode> AIBehaviors::GetFocusBuildingAIBehavior(Entity entity, Blackboard* blackboard)
{
	std::shared_ptr<BT::FallbackNode> root = std::make_shared<BT::FallbackNode>(BT::FallbackNode("root"));

	auto fallback1 = std::make_shared<BT::FallbackNode>(BT::FallbackNode("fallback1"));
	auto fallback2 = std::make_shared<BT::FallbackNode>(BT::FallbackNode("fallback2"));
	auto fallback3 = std::make_shared<BT::FallbackNode>(BT::FallbackNode("fallback3"));
	auto seq1 = std::make_shared<BT::SequenceNode>(BT::SequenceNode("seq1"));
	auto seq2 = std::make_shared<BT::SequenceNode>(BT::SequenceNode("seq2"));
	auto seq3 = std::make_shared<BT::SequenceNode>(BT::SequenceNode("seq3"));


	const float maxAggro = 600.f;
	const float closeRange = 30.f;

	//Try to find target that is close to AI
	auto findClosePlayerTarget = std::make_shared<BT::TargetPlayerCBT>(BT::TargetPlayerCBT("FindClosePlayerTarget", entity, blackboard, closeRange));
	auto findCloseBuildingTarget = std::make_shared<BT::TargetHouseCBT>(BT::TargetHouseCBT("FindCloseBuildingTarget", entity, blackboard, closeRange));
	auto findCloseDefenseTarget = std::make_shared<BT::TargetHouseCBT>(BT::TargetHouseCBT("FindCloseBuildingTarget", entity, blackboard, closeRange));

	//If nothing nearby was found, look across the map
	auto findBuildingTarget = std::make_shared<BT::TargetHouseCBT>(BT::TargetHouseCBT("FindBuildingTarget", entity, blackboard,  maxAggro));
	auto findDefenseTarget = std::make_shared<BT::TargetHouseCBT>(BT::TargetHouseCBT("FindBuildingTarget", entity, blackboard, maxAggro));
	auto findPlayer = std::make_shared<BT::TargetPlayerCBT>(BT::TargetPlayerCBT("FindTarget", entity, blackboard, maxAggro));

	auto moveToTarget = std::make_shared<BT::MoveCBT>(BT::MoveCBT("MoveToTarget", entity, blackboard));
	auto attackTarget = std::make_shared<BT::AttackCBT>(BT::AttackCBT("AttackTarget", entity, blackboard));
	auto inRange = std::make_shared<BT::InRangeCBT>(BT::InRangeCBT("InRange", entity, blackboard));
	auto genPath = std::make_shared<BT::GenPathCBT>(BT::GenPathCBT("GenPath", entity, blackboard));

	root->AddChild(fallback1);
	fallback1->AddChild(seq1);
	seq1->AddChild(fallback3);
	seq1->AddChild(fallback2);

	//Find a target
	fallback3->AddChild(findClosePlayerTarget);
	fallback3->AddChild(findCloseBuildingTarget);
	fallback3->AddChild(findCloseDefenseTarget);
	fallback3->AddChild(findPlayer);
	fallback3->AddChild(findBuildingTarget);
	fallback3->AddChild(findDefenseTarget);

	fallback2->AddChild(seq2);
	fallback2->AddChild(seq3);
	seq2->AddChild(inRange);
	seq2->AddChild(attackTarget);
	seq3->AddChild(genPath);
	seq3->AddChild(moveToTarget);

	return root;
}

std::shared_ptr<BT::FallbackNode> AIBehaviors::GetVillagerAIBehavior(Entity entity, Blackboard* blackboard)
{
	std::shared_ptr<BT::FallbackNode> root = std::make_shared<BT::FallbackNode>(BT::FallbackNode("root"));

	auto fallback1 = std::make_shared<BT::FallbackNode>(BT::FallbackNode("fallback1"));
	auto seq1 = std::make_shared<BT::SequenceNode>(BT::SequenceNode("seq1"));


	auto villagerTarget = std::make_shared<BT::VillagerTargetNodeCBT>(BT::VillagerTargetNodeCBT("VillagerTarget", entity, blackboard));
	auto moveToTarget = std::make_shared<BT::MoveCBT>(BT::MoveCBT("MoveToTarget", entity, blackboard));
	auto genPath = std::make_shared<BT::GenPathCBT>(BT::GenPathCBT("GenPath", entity, blackboard));
	auto hideVillager = std::make_shared<BT::HideVillagerCBT>(BT::HideVillagerCBT("HideVillager", entity, blackboard));

	root->AddChild(fallback1);
	fallback1->AddChild(hideVillager);
	fallback1->AddChild(seq1);
	seq1->AddChild(villagerTarget);
	seq1->AddChild(genPath);
	seq1->AddChild(moveToTarget);


	return root;
}
