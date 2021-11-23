#include "EnginePCH.h"
#include "FindBuildingTargetCBT.h"

BT::FindBuildingTargetCBT::FindBuildingTargetCBT(const std::string& name, Entity entity)
	:ActionNode(name),
	entity(entity)
{
}

BT::NodeStatus BT::FindBuildingTargetCBT::Tick()
{
	std::unordered_map<Entity, Entity> defenseEntities = Blackboard::Get().GetPathFindManager()->GetDefenseEntities();
	comp::Transform* transform = this->entity.GetComponent<comp::Transform>();
	Entity currentTarget;
	
	for (auto defenseEntity : defenseEntities)
	{
		if (currentTarget.IsNull())
			currentTarget = defenseEntity.second;

		else if(sm::Vector3::Distance(transform->position, defenseEntity.second.GetComponent<comp::Transform>()->position) <
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
