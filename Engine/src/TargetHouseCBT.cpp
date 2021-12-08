#include "EnginePCH.h"
#include "TargetHouseCBT.h"

BT::TargetHouseCBT::TargetHouseCBT(const std::string& name, Entity entity, Blackboard* blackboard, float aggroRange)
	:ActionNode(name),
	entity(entity),
	aggroRange(aggroRange),
	blackboard(blackboard)
{
}

BT::NodeStatus BT::TargetHouseCBT::Tick()
{
	//Get all houses in current game
	const Houses_t* housesEntities = blackboard->GetValue<Houses_t>("houses");
	//Get AI's transform component
	comp::Transform* transform = this->entity.GetComponent<comp::Transform>();

	if (housesEntities)
	{
		Entity currentTarget;
		//iterate through all potential houses
		for (auto house : housesEntities->houses)
		{
			//Need OBB for houses it holds the correct world position in Center vector
			comp::OrientedBoxCollider* houseOBB = house.second.GetComponent<comp::OrientedBoxCollider>();

			//Check for nullptr
			if (houseOBB == nullptr || !house.second.GetComponent<comp::House>()->homeNode)
				continue;

			//Need house position to be at 0.0 in Y axis, not to mess with distance. 
			sm::Vector3 housePosition = houseOBB->Center;
			housePosition.y = 0.0f;

			//Get the required components
			comp::Health* houseHealth = house.second.GetComponent<comp::Health>();
			comp::House* houseComp = house.second.GetComponent<comp::House>();

			//If missing components skip
			if (houseOBB == nullptr || houseHealth == nullptr || houseComp == nullptr)
				continue;

			//If no target has been set, take the first one close enough for the aggro range and is alive
			if (currentTarget.IsNull() && houseHealth->isAlive && houseComp->homeNode->reachable && sm::Vector3::Distance(transform->position, houseOBB->Center) < aggroRange)
			{
				currentTarget = house.second;
			}

			//If another house is closer change to it
			else if (!currentTarget.IsNull() && sm::Vector3::Distance(transform->position, housePosition) <
				sm::Vector3::Distance(transform->position, currentTarget.GetComponent<comp::OrientedBoxCollider>()->Center) && houseComp->homeNode->reachable)
			{
				currentTarget = house.second;
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
