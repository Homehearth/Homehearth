#include "EnginePCH.h"
#include "FindBuildingTargetCBT.h"

BT::FindBuildingTargetCBT::FindBuildingTargetCBT(const std::string& name, Entity entity)
	:ActionNode(name),
	entity(entity),
	refreshRate(1.0f)
{
	changeTargetTimer.Start();
}

BT::NodeStatus BT::FindBuildingTargetCBT::Tick()
{
	std::unordered_map<Entity, Entity> defenseEntities = Blackboard::Get().GetPathFindManager()->GetDefenseEntities();
	comp::Transform* transform = this->entity.GetComponent<comp::Transform>();
	Entity currentTarget;

	const PlayersPosition_t* playersEntity = Blackboard::Get().GetValue<PlayersPosition_t>("players");
	if (playersEntity)
	{
		currentTarget = playersEntity->players.at(0);
		//If a player is close enough target i
		for (auto player : playersEntity->players)
		{
			comp::Transform* playerTransform = player.GetComponent<comp::Transform>();
			if (playerTransform == nullptr)
				continue;

			if (currentTarget.IsNull() && sm::Vector3::Distance(transform->position, player.GetComponent<comp::Transform>()->position) < 20.0f)
			{
				currentTarget = player;
				return BT::NodeStatus::SUCCESS;
			}
		}
	}
	//Check all defenses and take the closest one
	for (auto defenseEntity : defenseEntities)
	{
		if (currentTarget.IsNull())
		{
			currentTarget = defenseEntity.second;
			continue;
		}

		if (defenseEntity.second.IsNull())
		{
			Blackboard::Get().GetPathFindManager()->RemoveDefenseEntity(defenseEntity.second);
			continue;
		}

		if(sm::Vector3::Distance(transform->position, defenseEntity.second.GetComponent<comp::Transform>()->position) <
				sm::Vector3::Distance(transform->position, currentTarget.GetComponent<comp::Transform>()->position))
		{
			currentTarget = defenseEntity.second;
		}
	}

	//If no defense building could be found
	if(currentTarget.IsNull())
	{
		return BT::NodeStatus::FAILURE;
	}


	//Add this entity target to blackboard
	Blackboard::Get().AddValue("target" + std::to_string(entity), currentTarget);

	return BT::NodeStatus::SUCCESS;
}
