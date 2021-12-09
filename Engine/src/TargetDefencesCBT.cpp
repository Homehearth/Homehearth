#include "EnginePCH.h"
#include "TargetDefencesCBT.h"

BT::TargetDefencesCBT::TargetDefencesCBT(const std::string& name, Entity entity, Blackboard* blackboard, float aggroRange)
	:ActionNode(name),
	entity(entity),
	aggroRange(aggroRange),
	blackboard(blackboard)
{
}

BT::NodeStatus BT::TargetDefencesCBT::Tick()
{
	//Get all defense entities in current game
	std::unordered_map<Entity, Entity>* defenseEntities = blackboard->GetPathFindManager()->GetDefenseEntities();
	//Get AI's transform component
	const comp::Transform* transform = this->entity.GetComponent<comp::Transform>();

	if(defenseEntities)
	{
		Entity currentTarget;
		//Check all defenses and take the closest one
		for (auto defenseEntity : *defenseEntities)
		{
			//if a value inside the map is null remove it from the system
			if (defenseEntity.second.IsNull())
			{
				blackboard->GetPathFindManager()->RemoveDefenseEntity(defenseEntity.second);
				continue;
			}
			//If no target init it if it's in aggroRrange
			if (currentTarget.IsNull() && sm::Vector3::Distance(transform->position, defenseEntity.second.GetComponent<comp::Transform>()->position) < aggroRange)
			{
				currentTarget = defenseEntity.second;
				continue;
			}
			//if the defense closer then the current target change to it.
			if (!currentTarget.IsNull() && sm::Vector3::Distance(transform->position, defenseEntity.second.GetComponent<comp::Transform>()->position) <
				sm::Vector3::Distance(transform->position, currentTarget.GetComponent<comp::Transform>()->position))
			{
				currentTarget = defenseEntity.second;
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
