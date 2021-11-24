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
	const PlayersPosition_t* playersEntity = Blackboard::Get().GetValue<PlayersPosition_t>("players");
	const comp::Transform* transform = entity.GetComponent<comp::Transform>();

	if(playersEntity == nullptr)
	{
		LOG_ERROR("Failed to get players entity");
		return BT::NodeStatus::FAILURE;
	}

	if(transform == nullptr)
	{
		LOG_ERROR("The enemy have no transform");
		return BT::NodeStatus::FAILURE;
	}
	PathFinderManager* aihandler = Blackboard::Get().GetPathFindManager();

	//Find the nearest player and set that position as target
	Entity nearestPlayer;
	for(auto player : playersEntity->players)
	{
		if(player.GetComponent<comp::Health>()->isAlive)
		{
			nearestPlayer = player;
			break;
		}
	}
	if (nearestPlayer.IsNull())
		return BT::NodeStatus::FAILURE;

	for (auto player : playersEntity->players)
	{
		comp::Transform* currentNearest = nearestPlayer.GetComponent<comp::Transform>();
		comp::Transform* playerTransform = player.GetComponent<comp::Transform>();
		comp::Health* playerHealth = player.GetComponent<comp::Health>();

		if(sm::Vector3::Distance(transform->position, playerTransform->position) < sm::Vector3::Distance(transform->position, currentNearest->position)
			&& aihandler->FindClosestNode(playerTransform->position)->reachable && playerHealth->isAlive)
		{
			nearestPlayer = player;
		}
	}

	if(aihandler->FindClosestNode(nearestPlayer.GetComponent<comp::Transform>()->position)->reachable)
	{
		Blackboard::Get().AddValue(("target" + std::to_string(entity)), nearestPlayer);
	}

	return BT::NodeStatus::SUCCESS;
}
