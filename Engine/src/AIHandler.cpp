#include "EnginePCH.h"
#include "AIHandler.h"


Entity AIHandler::FindClosestPlayer(HeadlessScene& scene, sm::Vector3 position, comp::NPC* npc)
{
	comp::Transform* transformCurrentClosest = nullptr;

	scene.ForEachComponent < comp::Player>([&](Entity& playerEntity, comp::Player& player)
		{
			if (!npc->currentClosest.IsNull())
			{
				transformCurrentClosest = npc->currentClosest.GetComponent<comp::Transform>();
				comp::Transform* transformPlayer = playerEntity.GetComponent<comp::Transform>();
				if (sm::Vector3::Distance(transformPlayer->position, position) < sm::Vector3::Distance(transformCurrentClosest->position, position))
				{
					LOG_INFO("Switching player");
					npc->currentClosest = playerEntity;
					transformCurrentClosest = npc->currentClosest.GetComponent<comp::Transform>();
				}
			}
			else
			{
				npc->currentClosest = playerEntity;
				transformCurrentClosest = npc->currentClosest.GetComponent<comp::Transform>();
			}
		});
	return npc->currentClosest;
}

Node* AIHandler::FindClosestNode(sm::Vector3 position)
{
	Node* currentClosest = nullptr;

	for (int j = 0; j < m_nodes.size(); j++)
	{
		for (int i = 0; i < m_nodes[j].size(); i++)
		{
			if (currentClosest)
			{
				if (sm::Vector3::Distance(currentClosest->position, position) > sm::Vector3::Distance(position, m_nodes[i][j]->position))
				{
					currentClosest = m_nodes[i][j].get();
				}
			}
			else
			{
				currentClosest = m_nodes[i][j].get();
			}
		}
	}

	return currentClosest;
}

std::vector<std::vector<std::shared_ptr<Node>>>& AIHandler::GetNodes()
{
	return m_nodes;
}

float AIHandler::CalculateFGH(Node* currentNode, Node* startNode, Node* goalNode)
{
	float tempG = currentNode->g + (currentNode->position - startNode->position).Length();
	float tempH = (currentNode->position - goalNode->position).Length(); //Using euclidean distance
	return tempG + tempH;
}

bool AIHandler::IsInVector(std::vector<Node*> vector, Node* node)
{
	for (Node* current : vector)
	{
		if (current == node)
		{
			return true;
		}
	}
	return false;
}

std::vector<Node*> AIHandler::GetNeighbors(GridSystem* grid, Tile* baseNode)
{
	std::vector<Node*> neighbors;
	if ((baseNode->type == TileType::DEFAULT || baseNode->type == TileType::EMPTY))
	{
		Tile* currentTile = nullptr;

		for (int newX = -1; newX <= 1; newX++)
		{
			for (int newY = -1; newY <= 1; newY++)
			{
				if (baseNode->gridID.x + newX >= grid->GetGridSize().x || baseNode->gridID.y + newY >= grid->GetGridSize().y
					|| baseNode->gridID.x + newX < 0 || baseNode->gridID.y + newY < 0)
					continue;

				if (newX == 0 && newY == 0)
				{
					continue;
				}

				//if(baseNode->gridID.x + newX == 38 && baseNode->gridID.y + newY == 20)
				//{
				//	__debugbreak();
				//}

				currentTile = grid->GetTile(Vector2I(baseNode->gridID.x + newX, baseNode->gridID.y + newY));

				if (currentTile->type == TileType::DEFAULT || currentTile->type == TileType::EMPTY)
				{
					neighbors.push_back(AddNode(currentTile->gridID));
				}

			}
		}



		//for (int i = max(0,  - 1); i <= min(baseNode->gridID.x + 1, grid->GetGridSize().x - 1); i++)
		//{
		//	for (int j = max(0, baseNode->gridID.y - 1); j <= min(baseNode->gridID.y + 1, grid->GetGridSize().y - 1); j++)
		//	{
		//		if (i != baseNode->gridID.x || j != baseNode->gridID.y)
		//		{
		//			currentTile = grid->GetTile(Vector2I(j, i));
		//			if (currentTile->type == TileType::DEFAULT
		//				|| currentTile->type == TileType::EMPTY)
		//			{
		//				neighbors.push_back(GetNodeByID({ j,i }));
		//			}
		//		}
		//	}
		//}


		//LOG_INFO("Connections: %d", nodes.at(i)->connections.size());
	}
	return neighbors;
}

Node* AIHandler::GetNodeByID(Vector2I id) const
{

	return m_nodes[id.x][id.y].get();

}

Node* AIHandler::AddNode(Vector2I id)
{
	if (GetNodeByID(id))
	{
		return GetNodeByID(id);
	}
	else
	{
		m_nodes[id.x][id.y] = std::make_unique<Node>(id);
		return m_nodes[id.x][id.y].get();
	}
}

bool AIHandler::AddNode(std::unique_ptr<Node>& node)
{
	if (GetNodeByID(node->id))
	{
		return false;
	}
	else
	{
		m_nodes[node->id.x][node->id.y] = std::move(node);
		return true;
	}
}

void AIHandler::CreateNodes(GridSystem* grid)
{
	int itrID = 0;
	//Create Nodes
	LOG_INFO("AIHandler: Creating Nodes");
	m_nodes.resize(grid->GetGridSize().x);
	for (int i = 0; i < m_nodes.size(); i++)
	{
		m_nodes[i].resize(grid->GetGridSize().y);
	}

	for (int j = 0; j < grid->GetGridSize().x; j++)
	{
		for (int i = 0; i < grid->GetGridSize().y; i++)
		{
			Tile* tile = grid->GetTile(Vector2I(j,i));

			Vector2I nodeID = tile->gridID;

			Node* currentNode = AddNode(tile->gridID);
			currentNode->position = tile->position;
			if (tile->type == TileType::DEFENCE || tile->type == TileType::UNPLACABLE ||
				tile->type == TileType::BUILDING)
			{
				currentNode->reachable = false;
			}
			else
			{
				std::vector<Node*> neighbors = GetNeighbors(grid, tile);
				for (auto neighbor : neighbors)
				{
					
					if (neighbor->reachable)
						m_nodes[j][i]->connections.push_back(neighbor);
				}
			}


		}
	}
	//Build Connections
	//LOG_INFO("AIHandler: Connecting Nodes");
	//for (int y = 0; y < grid->GetGridSize().y; y++)
	//{
	//	for (int x = 0; x < grid->GetGridSize().x; x++)
	//	{
	//		Tile* entityTile = grid->GetTile(Vector2I(y, x));

	//		if (entityTile->type == TileType::EMPTY || entityTile->type == TileType::DEFAULT)
	//		{
	//			std::vector<Node*> neighbors = GetNeighbors(grid, entityTile);
	//			for (auto neighbor : neighbors)
	//			{
	//				if (neighbor->reachable)
	//					m_nodes[x][y]->connections.push_back(neighbor);
	//			}
	//		}
	//		else
	//		{
	//			m_nodes[x][y]->reachable = false;
	//		}


	//	}
	//}


}

AIHandler::AIHandler()
{
}

AIHandler::~AIHandler()
{
	m_nodes.clear();
}

void AIHandler::SetClosestNode(comp::NPC& npc, sm::Vector3 position)
{
	npc.currentNode = FindClosestNode(position);
}

void AIHandler::AStarSearch(HeadlessScene& scene, Entity npc)
{
	comp::Transform* npcTransform = npc.GetComponent<comp::Transform>();
	comp::NPC* npcComp = npc.GetComponent<comp::NPC>();
	npcComp->path.clear();

	npcComp->currentNode->g = 0.0f;
	npcComp->currentNode->h = 0.0f;
	std::vector<Node*> openList, closedList;
	Node* startingNode = npcComp->currentNode;
	startingNode->f = 0.0f;
	openList.push_back(startingNode);

	Entity closestPlayer = FindClosestPlayer(scene, npcTransform->position, npcComp);
	comp::Transform* playerTransform = closestPlayer.GetComponent<comp::Transform>();
	Node* goalNode = FindClosestNode(playerTransform->position);

	while(!openList.empty())
	{
		Node* currentNode = openList.at(0);
		int ind = 0;
		for (int i = 1; i < openList.size(); i++)
		{
			if(openList[i]->f < currentNode->f)
			{
				currentNode = openList[i];
				ind = i;
			}
		}
		openList.erase(openList.begin() + ind);
		closedList.emplace_back(currentNode);

		if(currentNode == goalNode)
		{
			//Trace the path back
			if (goalNode->parent)
			{
				while (currentNode != startingNode)
				{
					npcComp->path.push_back(currentNode);
					currentNode = currentNode->parent;
				}
			}

			for (int i = 0; i < m_nodes.size(); i++)
			{
				for (int j = 0; j < m_nodes[i].size(); j++)
				{
					m_nodes[i][j]->parent = nullptr;
					m_nodes[i][j]->ResetFGH();
				}
			}
			return;
		}

		for (auto neighbour : currentNode->connections)
		{
			float gNew, hNew, fNew;
			if(IsInVector(closedList, neighbour))
			{
				continue;
			}

			gNew = currentNode->g + sm::Vector3::Distance(neighbour->position, currentNode->position);
			hNew = sm::Vector3::Distance(goalNode->position, neighbour->position);
			fNew = gNew + hNew;

			if(gNew < currentNode->g || !IsInVector(openList, neighbour))
			{
				neighbour->g = gNew;
				neighbour->h = hNew;
				neighbour->f = fNew;

				neighbour->parent = currentNode;

				openList.emplace_back(neighbour);
			}
		}
	}

	for (int i = 0; i < m_nodes.size(); i++)
	{
		for (int j = 0; j < m_nodes[i].size(); j++)
		{
			m_nodes[i][j]->parent = nullptr;
			m_nodes[i][j]->ResetFGH();
		}
	}
}

bool AIHandler::ReachedNode(const Entity npc)
{
	comp::NPC* npcComp = npc.GetComponent<comp::NPC>();
	comp::Transform* transformComp = npc.GetComponent<comp::Transform>();
	if (npcComp->currentNode && sm::Vector3::Distance(transformComp->position, npcComp->currentNode->position) < .2f)
	{
		return true;
	}
	else
	{
		return false;
	}
}
