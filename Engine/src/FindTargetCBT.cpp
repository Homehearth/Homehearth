#include "EnginePCH.h"
#include "FindTargetCBT.h"

BT::FindTargetCBT::FindTargetCBT(const std::string& name, Entity entity)
	:ActionNode(name),
	entity(entity)
{
}

BT::FindTargetCBT::~FindTargetCBT() = default;

BT::NodeStatus BT::FindTargetCBT::Tick()
{
	const PlayersPosition_t* playersPos = Blackboard::Get().GetValue<PlayersPosition_t>("playersPosition");
	const comp::Transform* transform = entity.GetComponent<comp::Transform>();

	if(playersPos == nullptr)
	{
		LOG_ERROR("Failed to get player positions");
		return BT::NodeStatus::FAILURE;
	}

	if(transform == nullptr)
	{
		LOG_ERROR("The enemy have no transform");
		return BT::NodeStatus::FAILURE;
	}
	Blackboard::Get().GetAIHandler();
	//Find the nearest player and set that position as target
	sm::Vector3 nearestPlayerPos = playersPos->positions.at(0);
	for (auto playerPos : playersPos->positions)
	{
		if(sm::Vector3::Distance(transform->position, playerPos) < sm::Vector3::Distance(transform->position, nearestPlayerPos))
		{
			nearestPlayerPos = playerPos;
		}
	}

	Blackboard::Get().AddValue(("target" + std::to_string(entity)), nearestPlayerPos);

	return BT::NodeStatus::SUCCESS;
}
