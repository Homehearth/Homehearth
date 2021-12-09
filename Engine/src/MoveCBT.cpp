#include "EnginePCH.h"
#include "MoveCBT.h"

BT::MoveCBT::MoveCBT(const std::string& name, Entity entity, Blackboard* blackboard)
	:ActionNode(name),
	entity(entity),
	refreshRateOnEscape(1.0f),
	blackboard(blackboard)
{
	timerEscape.Start();
}


bool BT::MoveCBT::EscapeCurrentNode(const Entity entity)
{
	comp::NPC* npc = entity.GetComponent<comp::NPC>();
	comp::Villager* villagerComp = entity.GetComponent<comp::Villager>();
	comp::Velocity* velocity = entity.GetComponent<comp::Velocity>();
	comp::Transform* transform = entity.GetComponent<comp::Transform>();

	if (!npc)
	{
		return false;
	}

	const std::vector<std::vector<std::shared_ptr<Node>>> nodes = blackboard->GetPathFindManager()->GetNodes();
	const int currentX = npc->currentNode->id.x;
	const int currentY = npc->currentNode->id.y;
	Node* currentClosest = nullptr;

	//For each neighbor starting from nort-west to south east
	//Find the closest node that is alive (Has connections with the grid)
	for (int newX = -1; newX <= 1; newX++)
	{
		for (int newY = -1; newY <= 1; newY++)
		{
			Node* neighborNode = nodes[currentX + newX][currentY + newY].get();

			if (currentX + newX >= 0 && currentY + newY >= 0 && neighborNode->reachable)
			{
				if (currentClosest)
				{
					//If it's enemy choice path from NPC component
					if(npc)
					{
						if (sm::Vector3::Distance(neighborNode->position, npc->currentNode->position) < sm::Vector3::Distance(currentClosest->position, npc->currentNode->position))
						{
							currentClosest = neighborNode;
						}
					}
					//Else need to take out path from villager component
					else if(villagerComp)
					{
						if (sm::Vector3::Distance(neighborNode->position, villagerComp->currentNode->position) < sm::Vector3::Distance(currentClosest->position, villagerComp->currentNode->position))
						{
							currentClosest = neighborNode;
						}
					}
				}
				else
				{
					currentClosest = nodes[currentX + newX][currentY + newY].get();
				}
			}
		}
	}
	//Adjust velocity to target the closest node that is alive.
	if (currentClosest)
	{
		velocity->vel = currentClosest->position - transform->position;
		velocity->vel.Normalize();

		if(npc)
			velocity->vel *= npc->movementSpeed;
		else if(villagerComp)
			velocity->vel *= villagerComp->movementSpeed;

		return true;
	}
	return false;
}

BT::NodeStatus BT::MoveCBT::Tick()
{
	comp::Transform* transform = entity.GetComponent<comp::Transform>();
	comp::Velocity* velocity = entity.GetComponent<comp::Velocity>();
	comp::NPC* npc = entity.GetComponent<comp::NPC>();
	comp::Villager* villager = entity.GetComponent<comp::Villager>();

	if (transform == nullptr || velocity == nullptr || npc == nullptr && villager == nullptr)
	{
		LOG_ERROR("Components returned as nullptr...");
		return BT::NodeStatus::FAILURE;
	}

	std::vector<Node*>* path = nullptr;
	Node* currentNode = nullptr;
	if (npc)
	{
		path = &npc->path;
		currentNode = npc->currentNode;
	}
	else if (villager)
	{
		path = &villager->path;
		currentNode = villager->currentNode;
	}

	if (currentNode && path->empty())
	{
		Entity* target = blackboard->GetValue<Entity>("target" + std::to_string(entity));
		sm::Vector3* villagerTarget = blackboard->GetValue<sm::Vector3>("villagerTarget" + std::to_string(entity));
		comp::House* house = nullptr;

		if(target)
			house = target->GetComponent<comp::House>();

		sm::Vector3* targetPosition = nullptr;

		if (target)
			targetPosition = &target->GetComponent<comp::Transform>()->position;
		else if (villagerTarget)
			targetPosition = villagerTarget;

		if (!targetPosition)
			return BT::NodeStatus::FAILURE;

		PathFinderManager* pathFindManager = blackboard->GetPathFindManager();
		//Check if AI stands on a dead node (no connections to grid)
		if (!currentNode->reachable && !pathFindManager->FindClosestNode(*targetPosition)->defencePlaced && timerEscape.GetElapsedTime<std::chrono::seconds>() > refreshRateOnEscape)
		{
			timerEscape.Start();
			if (EscapeCurrentNode(entity))
			{
				return BT::NodeStatus::SUCCESS;
			}

			LOG_WARNING("Standing on non rechable node and failed to generate escape path");
			return BT::NodeStatus::FAILURE;
		}
		//If target is defense move the last distance to it (node is not reachable by A*)
		else if(pathFindManager->FindClosestNode(*targetPosition)->defencePlaced && targetPosition)
		{
			velocity->vel = *targetPosition - transform->position;
			velocity->vel.Normalize();
			if(npc)
				velocity->vel *= npc->movementSpeed;
			else if(villager)
				velocity->vel *= villager->movementSpeed;
		}
		else if(house != nullptr)
		{
			comp::OrientedBoxCollider* obb = entity.GetComponent<comp::OrientedBoxCollider>();
			if (obb == nullptr)
				return BT::NodeStatus::FAILURE;

			sm::Vector3 position = obb->Center;
			velocity->vel = position - transform->position;
			velocity->vel.Normalize();
			if(npc)
				velocity->vel *= npc->movementSpeed;
			else if(villager)
				velocity->vel *= villager->movementSpeed;
		}
		else if (currentNode->reachable)
		{
			velocity->vel = sm::Vector3(0.0f, 0.0f, 0.0f);
		}

		return BT::NodeStatus::SUCCESS;
	}
	else
	{
		currentNode = path->back();
		if (velocity && currentNode)
		{
			velocity->vel = currentNode->position - transform->position;
			velocity->vel.Normalize();
			if(npc)
				velocity->vel *= npc->movementSpeed;
			else if (villager)
				velocity->vel *= villager->movementSpeed;


			//Update animation depending on velocity
			comp::AnimationState* animState = entity.GetComponent<comp::AnimationState>();
			if (animState)
			{
				if (velocity->vel.Length() > 0.01f)
					animState->toSend = EAnimationType::MOVE;
				else
					animState->toSend = EAnimationType::IDLE;
			}
		}

		//If AI close enough to next node, pop it from the path
		if (sm::Vector3::Distance(currentNode->position, transform->position) < 1.0f)
		{
			path->pop_back();
		}

		return BT::NodeStatus::SUCCESS;
	}

}
