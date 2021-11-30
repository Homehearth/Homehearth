#include "EnginePCH.h"
#include "FindBuildingTargetCBT.h"

BT::FindBuildingTargetCBT::FindBuildingTargetCBT(const std::string& name, Entity entity)
	:ActionNode(name),
	entity(entity),
	refreshRate(1.0f),
	aggroRange(20.f)
{
	changeTargetTimer.Start();
}

BT::NodeStatus BT::FindBuildingTargetCBT::Tick()
{
	//Get all players in current game
	const PlayersPosition_t* playersEntity = Blackboard::Get().GetValue<PlayersPosition_t>("players");
	//Get all houses in current game
	const Houses_t* housesEntities = Blackboard::Get().GetValue<Houses_t>("houses");
	//Get all defense entities in current game
	std::unordered_map<Entity, Entity> defenseEntities = Blackboard::Get().GetPathFindManager()->GetDefenseEntities();
	//Get AI's transform component
	comp::Transform* transform = this->entity.GetComponent<comp::Transform>();

	Entity currentTarget;
	if (playersEntity)
	{
		//If a player is close enough target it
		for (auto player : playersEntity->players)
		{
			comp::Transform* playerTransform = player.GetComponent<comp::Transform>();
			comp::Health* playerHealth = player.GetComponent<comp::Health>();
			//If missing components skip
			if (playerTransform == nullptr || playerHealth == nullptr)
				continue;
			//If no target has been set, take the first one close enough for the aggro range and is alive
			if (currentTarget.IsNull() && sm::Vector3::Distance(transform->position, player.GetComponent<comp::Transform>()->position) < aggroRange && playerHealth->isAlive)
			{
				currentTarget = player;
			}
			//If another player is closer change to it
			else if(!currentTarget.IsNull() && sm::Vector3::Distance(transform->position, player.GetComponent<comp::Transform>()->position) < 
				sm::Vector3::Distance(transform->position, currentTarget.GetComponent<comp::Transform>()->position))
			{
				currentTarget = player;
			}
		}

		//If target was found return success
		if(!currentTarget.IsNull())
		{
			Blackboard::Get().AddValue("target" + std::to_string(entity), currentTarget);
			return BT::NodeStatus::SUCCESS;
		}

	}

	//Check Houses
	if (housesEntities)
	{
		//If a player is close enough target it
		for (auto house : housesEntities->houses)
		{
			//Need OBB for houses it holds the correct world position in Center vector
			comp::OrientedBoxCollider* houseOBB = house.second.GetComponent<comp::OrientedBoxCollider>();
			if (houseOBB == nullptr)
				continue;

			sm::Vector3 housePosition = houseOBB->Center;
			housePosition.y = 0.0f;
			comp::Health* houseHealth = house.second.GetComponent<comp::Health>();
			//If missing components skip
			if (houseOBB == nullptr || houseHealth == nullptr)
				continue;
			//If no target has been set, take the first one close enough for the aggro range and is alive
			if (currentTarget.IsNull() && sm::Vector3::Distance(transform->position, housePosition) < 40.f && houseHealth->isAlive)
			{
				currentTarget = house.second;
			}
			//If another house is closer change to it
			else if (!currentTarget.IsNull() && sm::Vector3::Distance(transform->position, housePosition) <
				sm::Vector3::Distance(transform->position, currentTarget.GetComponent<comp::Transform>()->position))
			{
				currentTarget = house.second;
			}
		}

		//If target was found return success
		if (!currentTarget.IsNull())
		{
			Blackboard::Get().AddValue("target" + std::to_string(entity), currentTarget);
			return BT::NodeStatus::SUCCESS;
		}

	}


	//Check all defenses and take the closest one
	for (auto defenseEntity : defenseEntities)
	{
		//If no target init it
		if (currentTarget.IsNull())
		{
			currentTarget = defenseEntity.second;
			continue;
		}

		//if a value inside the map is null remove it from the system
		if (defenseEntity.second.IsNull())
		{
			Blackboard::Get().GetPathFindManager()->RemoveDefenseEntity(defenseEntity.second);
			continue;
		}

		//if the target closer then the current one change to it.
		if(sm::Vector3::Distance(transform->position, defenseEntity.second.GetComponent<comp::Transform>()->position) <
				sm::Vector3::Distance(transform->position, currentTarget.GetComponent<comp::Transform>()->position))
		{
			currentTarget = defenseEntity.second;
		}
	}
	//If target was found return success
	if(!currentTarget.IsNull())
	{
		//Add this entity target to blackboard
		Blackboard::Get().AddValue("target" + std::to_string(entity), currentTarget);
		return BT::NodeStatus::SUCCESS;
	}

	//If no target was set return failure
	return BT::NodeStatus::FAILURE;
}
