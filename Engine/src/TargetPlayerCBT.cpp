#include "EnginePCH.h"
#include "TargetPlayerCBT.h"

BT::TargetPlayerCBT::TargetPlayerCBT(const std::string& name, Entity entity, Blackboard* blackboard, float aggroRange)
	:ActionNode(name),
	entity(entity),
	aggroRange(aggroRange),
	blackboard(blackboard)
{

}

BT::NodeStatus BT::TargetPlayerCBT::Tick()
{
	//Get all players in current game
	const PlayersPosition_t* playersEntity = blackboard->GetValue<PlayersPosition_t>("players");

	//Get AI's transform component
	const comp::Transform* transform = this->entity.GetComponent<comp::Transform>();

	Entity* targetEntity = blackboard->GetValue<Entity>("target" + std::to_string(entity));
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
			else if (!currentTarget.IsNull() && sm::Vector3::Distance(transform->position, player.GetComponent<comp::Transform>()->position) <
				sm::Vector3::Distance(transform->position, currentTarget.GetComponent<comp::Transform>()->position))
			{
				currentTarget = player;
			}
		}

		//If target was found return success
		if (!currentTarget.IsNull())
		{
			blackboard->AddValue("target" + std::to_string(entity), currentTarget);
			return BT::NodeStatus::SUCCESS;
		}

	}

	return BT::NodeStatus::FAILURE;
}
