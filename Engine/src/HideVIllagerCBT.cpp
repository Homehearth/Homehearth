#include "EnginePCH.h"
#include "HideVillagerCBT.h"

BT::HideVillagerCBT::HideVillagerCBT(const std::string& name, Entity entity)
	:ActionNode(name),
	entity(entity)
{
}

BT::NodeStatus BT::HideVillagerCBT::Tick()
{
	comp::Villager* villager = entity.GetComponent<comp::Villager>();
	comp::Transform* transform = entity.GetComponent<comp::Transform>();
	comp::Velocity* vel = entity.GetComponent<comp::Velocity>();
	CyclePeriod* cycle = Blackboard::Get().GetValue<CyclePeriod>("cycle");

	if(cycle == nullptr || villager == nullptr || transform == nullptr || vel == nullptr)
	{
		LOG_WARNING("Failed to get components/values...");
		return BT::NodeStatus::FAILURE;
	}

	if (*cycle == CyclePeriod::MORNING && villager->isFleeing)
	{
		entity.GetComponent<comp::Velocity>()->vel = sm::Vector3::Zero;
		return BT::NodeStatus::FAILURE;
	}

	if (*cycle == CyclePeriod::NIGHT && villager->isHiding && !villager->homeHouse.IsNull())
	{
		//Keep villager hidden by posting this as success
		return BT::NodeStatus::SUCCESS;
	}

	if(*cycle == CyclePeriod::NIGHT && !villager->isHiding && !villager->homeHouse.IsNull())
	{
		if(sm::Vector3::Distance(transform->position, villager->homeHouse.GetComponent<comp::House>()->attackNode->position) < 7.f)
		{
			//Hide the villager.
			transform->position.y = 999.f; //hides npc high in the sky!
			villager->isHiding = true;
			vel->vel = sm::Vector3(0.0f, 0.0f, 0.0f);
			entity.UpdateNetwork();
			return BT::NodeStatus::SUCCESS;
		}
	}
	if(*cycle == CyclePeriod::MORNING || *cycle == CyclePeriod::DAY && villager->isHiding)
	{
		//Pop villager out again.
		transform->position.y = 0.0f;
		villager->isHiding = false;
		entity.UpdateNetwork();
		return BT::NodeStatus::SUCCESS;
	}
	if(villager->homeHouse.IsNull() && villager->isHiding)
	{
		//Pop villager out again.
		transform->position.y = 0.0f;
		villager->isHiding = false;
		villager->isFleeing = true;
		entity.UpdateNetwork();
		return BT::NodeStatus::SUCCESS;
	}

	return BT::NodeStatus::FAILURE;
}
