#include "EnginePCH.h"
#include "GenPathCBT.h"
#include "PathFinderManager.h"

BT::GenPathCBT::GenPathCBT(const std::string& name, Entity entity)
	:ActionNode(name),
	entity(entity),
	refreshRate(0.2f)
{
	generatePathTimer.Start();
}

BT::NodeStatus BT::GenPathCBT::Tick()
{
	PathFinderManager* aiHandler = Blackboard::Get().GetPathFindManager();

	if(aiHandler != nullptr)
	{
		comp::NPC* npc = entity.GetComponent<comp::NPC>();
		npc->currentNode = aiHandler->FindClosestNode(entity.GetComponent<comp::Transform>()->position);

		if(npc->path.empty() || generatePathTimer.GetElapsedTime<std::chrono::seconds>() > refreshRate)
		{
			generatePathTimer.Start();
			
			aiHandler->AStarSearch(entity);
		}

		return BT::NodeStatus::SUCCESS;
	}
	else
	{
		LOG_ERROR("Failed to generate a path");
	}

	return BT::NodeStatus::FAILURE;
}
