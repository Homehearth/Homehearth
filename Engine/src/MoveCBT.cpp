#include "EnginePCH.h"
#include "MoveCBT.h"

BT::MoveCBT::MoveCBT(const std::string& name, Entity entity)
	:ActionNode(name),
	entity(entity),
	runNrTimes(600)
{
}


BT::NodeStatus BT::MoveCBT::Tick()
{
	//if(runNrTimes <= 0)
	{
		//Temp function to get the direction to target and add that to velocity
		comp::Transform* transform = entity.GetComponent<comp::Transform>();
		comp::Velocity* veloicty = entity.GetComponent<comp::Velocity>();
		comp::NPC* npc = entity.GetComponent<comp::NPC>();

		if(npc->path.empty())
		{
			LOG_ERROR("Path was empty");
			veloicty->vel = sm::Vector3(0.f, 0.f, 0.f);

			//PUT AI back on a reachable node
			if(!npc->currentNode->reachable)
			{
				std::vector<std::vector<std::shared_ptr<Node>>> nodes = Blackboard::Get().GetAIHandler()->GetNodes();
				int currentX = npc->currentNode->id.x;
				int currentY = npc->currentNode->id.y;
				Node* currentClosest = nullptr;
				//For each neighbour starting from nort-west to south east
				for (int newX = -1; newX <= 1; newX++)
				{
					for (int newY = -1; newY <= 1; newY++)
					{
						Node* neighborNode = nodes[currentX + newX][currentY + newY].get();

						if(currentX + newX >= 0 && currentY + newY >= 0 && neighborNode->reachable)
						{
							if(currentClosest)
							{
								if(sm::Vector3::Distance(neighborNode->position, npc->currentNode->position) < sm::Vector3::Distance(currentClosest->position, npc->currentNode->position))
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

				if(currentClosest)
				{
					veloicty->vel = currentClosest->position - transform->position;
					veloicty->vel.Normalize();
					veloicty->vel *= npc->movementSpeed;
					return BT::NodeStatus::SUCCESS;
				}

				LOG_ERROR("Standing on non rechable node and failed to generate escape path");
			}

			return BT::NodeStatus::FAILURE;
		}
		if (transform == nullptr)
		{
			LOG_ERROR("Transform component returned as nullptr...");
			return BT::NodeStatus::FAILURE;
		}

		npc->currentNode = npc->path.back();
		if (veloicty && npc->currentNode)
		{
			veloicty->vel = npc->currentNode->position - transform->position;
			veloicty->vel.Normalize();
			veloicty->vel *= npc->movementSpeed;
		}

		if (sm::Vector3::Distance(npc->currentNode->position, transform->position) < 4.f)
		{
			npc->path.pop_back();
		}


		//entity.GetComponent<comp::BehaviorTree>()->currentNode = this;

		//runNrTimes--;
		return BT::NodeStatus::SUCCESS;
	}
	//else
	//{
	//	LOG_INFO("Reseted currentNode");
	//	runNrTimes = 600;
	//	entity.GetComponent<comp::BehaviorTree>()->currentNode = nullptr;
	//	return BT::NodeStatus::SUCCESS;
	//}

}
