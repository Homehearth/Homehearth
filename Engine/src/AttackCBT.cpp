#include "EnginePCH.h"
#include "AttackCBT.h"

BT::AttackCBT::AttackCBT(const std::string& name, Entity entity)
	:ActionNode(name),
	entity(entity)
{
}

BT::NodeStatus BT::AttackCBT::Tick()
{
	comp::CombatStats* stats = entity.GetComponent<comp::CombatStats>();
	comp::Transform* transform = entity.GetComponent<comp::Transform>();
	Entity* target = Blackboard::Get().GetValue<Entity>("target" + std::to_string(entity));

	if(stats == nullptr || transform == nullptr)
	{
		LOG_ERROR("Failed to get components from entity");
		return BT::NodeStatus::FAILURE;
	}
	if(target == nullptr)
	{
		LOG_ERROR("Failed to get target position");
		return BT::NodeStatus::FAILURE;
	}

	//Perform the attack?
	PathFinderManager* aihandler = Blackboard::Get().GetAIHandler();

	stats->targetDir = target->GetComponent<comp::Transform>()->position - transform->position;
	stats->targetDir.Normalize();

	if (ecs::Use(stats))
	{
		// Enemy Attacked
	};

	return BT::NodeStatus::SUCCESS;
}
