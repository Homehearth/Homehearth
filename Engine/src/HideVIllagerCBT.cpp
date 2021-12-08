#include "EnginePCH.h"
#include "HideVillagerCBT.h"

BT::HideVillagerCBT::HideVillagerCBT(const std::string& name, Entity entity)
	:ActionNode(name),
	entity(entity)
{
}

BT::NodeStatus BT::HideVillagerCBT::Tick()
{
	//RETURN SUCCESS WHEN WE DON'T WANT THE SEQUENCE AFTER TO RUN.
	//RETURN FAILURE IF WE WANT NEXT NODES TO BE CHECKED.

	//Get required data
	comp::Villager* villager = entity.GetComponent<comp::Villager>();
	comp::Transform* transform = entity.GetComponent<comp::Transform>();
	comp::Velocity* vel = entity.GetComponent<comp::Velocity>();
	CyclePeriod* cycle = Blackboard::Get().GetValue<CyclePeriod>("cycle");

	//Nullptr check
	if(cycle == nullptr || villager == nullptr || transform == nullptr || vel == nullptr)
	{
		LOG_WARNING("Failed to get components/values...");
		return BT::NodeStatus::FAILURE;
	}

	//If morning/day and villager is in fleeing mode return to normal 
	if (*cycle == CyclePeriod::MORNING && villager->isFleeing || *cycle == CyclePeriod::DAY && villager->isFleeing)
	{
		vel->vel = sm::Vector3::Zero;
		villager->isFleeing = false;

		//Activate idle animation
		comp::AnimationState* animState = entity.GetComponent<comp::AnimationState>();
		if (animState)
			animState->toSend = EAnimationType::IDLE
		;

		return BT::NodeStatus::FAILURE;
	}

	//If night, villager is hiding and the home is safe do nothing
	if (*cycle == CyclePeriod::NIGHT && villager->isHiding && !villager->homeHouse.IsNull())
	{
		//Keep villager hidden by posting this as success
		return BT::NodeStatus::SUCCESS;
	}

	//If night, villager is not hiding and home is gone! turn on fleeing mode
	if (*cycle == CyclePeriod::NIGHT && !villager->isHiding && villager->homeHouse.IsNull())
	{
		//Keep villager hidden by posting this as success
		villager->isFleeing = true;
		return BT::NodeStatus::FAILURE;
	}

	//if night and villager not hiding and home is safe, try to hide in house!
	if(*cycle == CyclePeriod::NIGHT && !villager->isHiding && !villager->homeHouse.IsNull())
	{
		if(sm::Vector3::Distance(transform->position, villager->homeHouse.GetComponent<comp::House>()->homeNode->position) < 7.f)
		{
			//Hide the villager.
			transform->position.y = 999.f; //hides npc high in the sky!
			villager->isHiding = true;
			vel->vel = sm::Vector3(0.0f, 0.0f, 0.0f);
			entity.UpdateNetwork();
			return BT::NodeStatus::SUCCESS;
		}
	}

	//if morning/day and villager in hiding pop them out from house
	if(*cycle == CyclePeriod::MORNING && villager->isHiding || *cycle == CyclePeriod::DAY && villager->isHiding)
	{
		//Pop villager out again.
		transform->position.y = 0.0f;
		villager->isHiding = false;
		entity.UpdateNetwork();

		comp::AnimationState* animState = entity.GetComponent<comp::AnimationState>();
		//Activate idle animation
		if (animState)
			animState->toSend = EAnimationType::IDLE;

		return BT::NodeStatus::FAILURE;
	}

	//if villager in hiding pop them out from house
	if(villager->homeHouse.IsNull() && villager->isHiding)
	{
		//Pop villager out again.
		transform->position.y = 0.0f;
		villager->isHiding = false;
		villager->isFleeing = true;
		entity.UpdateNetwork();
		return BT::NodeStatus::FAILURE;
	}

	return BT::NodeStatus::FAILURE;
}
