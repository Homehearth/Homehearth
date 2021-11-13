#include "EnginePCH.h"
#include "GenPathCBT.h"
#include "AIHandler.h"

BT::GenPathCBT::GenPathCBT(const std::string& name, Entity entity)
	:ActionNode(name),
	entity(entity)
{
}

BT::NodeStatus BT::GenPathCBT::Tick()
{
	AIHandler* aiHandler = Blackboard::Get().GetAIHandler();

	if(aiHandler != nullptr)
	{
		entity.GetComponent<comp::NPC>()->currentNode = aiHandler->FindClosestNode(entity.GetComponent<comp::Transform>()->position);
		aiHandler->AStarSearch(entity);
		return BT::NodeStatus::SUCCESS;
	}
	else
	{
		LOG_ERROR("Failed to generate a path");
	}

	return BT::NodeStatus::FAILURE;
}
