#include "EnginePCH.h"
#include "GenPathCBT.h"
#include "PathFinderManager.h"

BT::GenPathCBT::GenPathCBT(const std::string& name, Entity entity, Blackboard* blackboard)
	:ActionNode(name),
	entity(entity),
	refreshRate(0.2f),
	blackboard(blackboard)
{
	generatePathTimer.Start();
}

BT::NodeStatus BT::GenPathCBT::Tick()
{
	PathFinderManager* aiHandler = blackboard->GetPathFindManager();

	if(aiHandler != nullptr)
	{
		comp::NPC* npc = entity.GetComponent<comp::NPC>();
		comp::Villager* villager = entity.GetComponent<comp::Villager>();
		if(npc)
		{
			npc->currentNode = aiHandler->FindClosestNode(entity.GetComponent<comp::Transform>()->position);

			if( (npc->path.empty() || generatePathTimer.GetElapsedTime<std::chrono::seconds>() > refreshRate))
			{
				generatePathTimer.Start();
				
				aiHandler->AStarSearch(entity, blackboard);
			}
			return BT::NodeStatus::SUCCESS;
		}
		else if(villager)
		{
			villager->currentNode = aiHandler->FindClosestNode(entity.GetComponent<comp::Transform>()->position);

			if ((villager->path.empty() || generatePathTimer.GetElapsedTime<std::chrono::seconds>() > refreshRate))
			{
				generatePathTimer.Start();

				aiHandler->AStarSearch(entity, blackboard);
			}
			return BT::NodeStatus::SUCCESS;
		}
	}
	else
	{
		LOG_ERROR("Failed to generate a path");
	}

	return BT::NodeStatus::FAILURE;
}
