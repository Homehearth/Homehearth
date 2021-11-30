#include "EnginePCH.h"
#include "MoveCBT.h"

BT::MoveCBT::MoveCBT(const std::string& name, Entity entity)
	:ActionNode(name),
	entity(entity),
	refreshRateOnEscape(1.0f)
{
	timerEscape.Start();
}


bool BT::MoveCBT::EscapeCurrentNode(const Entity entity)
{
	comp::NPC* npc = entity.GetComponent<comp::NPC>();
	comp::Velocity* velocity = entity.GetComponent<comp::Velocity>();
	comp::Transform* transform = entity.GetComponent<comp::Transform>();

	const std::vector<std::vector<std::shared_ptr<Node>>> nodes = Blackboard::Get().GetPathFindManager()->GetNodes();
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
					if (sm::Vector3::Distance(neighborNode->position, npc->currentNode->position) < sm::Vector3::Distance(currentClosest->position, npc->currentNode->position))
					{
						currentClosest = neighborNode;
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
		velocity->vel *= npc->movementSpeed;
		return true;
	}
	return false;
}

BT::NodeStatus BT::MoveCBT::Tick()
{
	comp::Transform* transform = entity.GetComponent<comp::Transform>();
	comp::Velocity* velocity = entity.GetComponent<comp::Velocity>();
	comp::NPC* npc = entity.GetComponent<comp::NPC>();

	if (transform == nullptr || velocity == nullptr || npc == nullptr)
	{
		LOG_ERROR("Components returned as nullptr...");
		return BT::NodeStatus::FAILURE;
	}


	if (npc->path.empty())
	{
		Entity* target = Blackboard::Get().GetValue<Entity>("target" + std::to_string(entity));
		comp::House* house = target->GetComponent<comp::House>();
		comp::Transform* targetTransform = nullptr;

		if(target)
			targetTransform = target->GetComponent<comp::Transform>();

		if (!targetTransform)
			return BT::NodeStatus::FAILURE;

		PathFinderManager* pathFindManager = Blackboard::Get().GetPathFindManager();
		//Check if AI stands on a dead node (no connections to grid)
		if (!npc->currentNode->reachable && !pathFindManager->FindClosestNode(targetTransform->position)->defencePlaced && timerEscape.GetElapsedTime<std::chrono::seconds>() > refreshRateOnEscape)
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
		else if(pathFindManager->FindClosestNode(target->GetComponent<comp::Transform>()->position)->defencePlaced && targetTransform)
		{
			velocity->vel = targetTransform->position - transform->position;
			velocity->vel.Normalize();
			velocity->vel *= npc->movementSpeed;
		}
		else if(house)
		{
			comp::OrientedBoxCollider* obb = entity.GetComponent<comp::OrientedBoxCollider>();
			if (obb == nullptr)
				return BT::NodeStatus::FAILURE;

			sm::Vector3 position = obb->Center;
			velocity->vel = position - transform->position;
			velocity->vel.Normalize();
			velocity->vel *= npc->movementSpeed;
		}
		else if (npc->currentNode->reachable)
		{
			velocity->vel = sm::Vector3(0.0f, 0.0f, 0.0f);
		}

		return BT::NodeStatus::SUCCESS;
	}
	else
	{
		npc->currentNode = npc->path.back();
		if (velocity && npc->currentNode)
		{
			velocity->vel = npc->currentNode->position - transform->position;
			velocity->vel.Normalize();
			velocity->vel *= npc->movementSpeed;


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
		if (sm::Vector3::Distance(npc->currentNode->position, transform->position) < 1.0f)
		{
			npc->path.pop_back();
		}

		return BT::NodeStatus::SUCCESS;
	}

}
