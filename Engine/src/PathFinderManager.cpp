#include "EnginePCH.h"
#include "PathFinderManager.h"

Node* PathFinderManager::FindClosestNode(sm::Vector3 position)
{
	Node* currentClosest = nullptr;

	int clampedX = static_cast<int>(abs(position.x) / m_nodeSize.x);
	int clampedZ = static_cast<int>(abs(position.z) / m_nodeSize.y);

	if (clampedX > 69)
		clampedX = 69;
	if (clampedZ > 69)
		clampedZ = 69;

	currentClosest = m_nodes[clampedZ][clampedX].get();

	return currentClosest;
}

float PathFinderManager::GetNodeSize() const
{
	return this->m_nodeSize.x;
}

std::vector<std::vector<std::shared_ptr<Node>>>& PathFinderManager::GetNodes()
{
	return m_nodes;
}

std::unordered_map<Entity, Entity> PathFinderManager::GetDefenseEntities()
{
	return defenseEntities;
}

void PathFinderManager::AddDefenseEntity(Entity entity)
{
	if(!entity.IsNull())
		defenseEntities.insert(std::make_pair(entity, entity));
}

void PathFinderManager::RemoveDefenseEntity(Entity entity)
{
	if(!entity.IsNull())
		defenseEntities.erase(entity);
}

bool PathFinderManager::IsInVector(std::vector<Node*> vector, Node* node)
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

std::vector<Node*> PathFinderManager::GetNeighbors(GridSystem* grid, Tile* baseNode)
{
	std::vector<Node*> neighbors;

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

			currentTile = grid->GetTile(Vector2I(baseNode->gridID.x + newX, baseNode->gridID.y + newY));

			neighbors.push_back(AddNode(currentTile->gridID));


		}
	}

	return neighbors;
}

Node* PathFinderManager::GetDistantNode(sm::Vector3 position)
{
	for (int i = 0; i < m_nodes.size(); i++)
	{
		for (int j = 0; j < m_nodes[i].size(); j++)
		{
			if (!m_nodes[i][j]->defencePlaced && m_nodes[i][j]->reachable && sm::Vector3::Distance(position, m_nodes[i][j]->position) >= 100)
			{
				return m_nodes[i][j].get();
			}
		}
	}
	return nullptr;
}

Node* PathFinderManager::GetNodeByID(Vector2I id) const
{

	return m_nodes[id.x][id.y].get();

}

Node* PathFinderManager::AddNode(Vector2I id)
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

void PathFinderManager::CreateNodes(GridSystem* grid)
{
	int itrID = 0;
	//Create Nodes
	LOG_INFO("PathFinderManager: Creating Nodes");
	m_nodes.resize(grid->GetGridSize().x);
	for (int i = 0; i < m_nodes.size(); i++)
	{
		m_nodes[i].resize(grid->GetGridSize().y);
	}

	for (int j = 0; j < grid->GetGridSize().x; j++)
	{
		for (int i = 0; i < grid->GetGridSize().y; i++)
		{
			Tile* tile = grid->GetTile(Vector2I(j, i));

			Vector2I nodeID = tile->gridID;

			Node* currentNode = AddNode(tile->gridID);
			currentNode->position = tile->position;
			if (tile->type == TileType::DEFENCE ||
				tile->type == TileType::UNPLACABLE ||
				tile->type == TileType::BUILDING)
			{
				currentNode->reachable = false;
			}

			std::vector<Node*> neighbors = GetNeighbors(grid, tile);
			for (auto neighbor : neighbors)
			{
				m_nodes[j][i]->connections.push_back(neighbor);
			}
		}
	}
	m_nodeSize = grid->m_tileSize;
}

PathFinderManager::PathFinderManager()
{
}

PathFinderManager::~PathFinderManager()
{
	m_nodes.clear();
}

void PathFinderManager::AStarSearch(Entity npc)
{
	comp::Transform* npcTransform = npc.GetComponent<comp::Transform>();
	comp::NPC* npcComp = npc.GetComponent<comp::NPC>();


	npcComp->currentNode->g = 0.0f;
	npcComp->currentNode->h = 0.0f;
	std::vector<Node*> openList, closedList;
	Node* startingNode = npcComp->currentNode;
	startingNode->f = 0.0f;
	openList.push_back(startingNode);

	//Gets the target that findTargetNode has picked for this entity
	Entity* target = Blackboard::Get().GetValue<Entity>("target" + std::to_string(npc));
	if (target == nullptr)
	{
		LOG_INFO("Target was nullptr...");
		return;
	}


	Node* goalNode = FindClosestNode(target->GetComponent<comp::Transform>()->position);

	//If goal is a defense
	if(goalNode->defencePlaced)
	{
		Node* currentNode = nullptr;
		//Go through all neighbours and find the one closest that is reachable
		for (auto& neighbour : goalNode->connections)
		{
			if(neighbour->reachable && currentNode == nullptr)
			{
				currentNode = neighbour;
			}
			else if(neighbour->reachable && sm::Vector3::Distance(npcTransform->position, neighbour->position) < sm::Vector3::Distance(npcTransform->position, currentNode->position))
			{
				currentNode = neighbour;
			}
		}
		if(currentNode == nullptr)
		{
			return;
		}

		goalNode = currentNode;
	}
	//If goal is a dead node cancel
	else if (!goalNode->reachable)
	{
		return;
	}

	npcComp->path.clear();
	while (!openList.empty())
	{
		Node* currentNode = openList.at(0);
		int ind = 0;
		for (int i = 1; i < openList.size(); i++)
		{
			if (openList[i]->f < currentNode->f)
			{
				currentNode = openList[i];
				ind = i;
			}
		}
		openList.erase(openList.begin() + ind);
		closedList.emplace_back(currentNode);

		if (currentNode == goalNode)
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

			for (Node* node : openList)
			{
				node->ResetFGH();
				node->parent = nullptr;
			}
			for (Node* node : closedList)
			{
				node->ResetFGH();
				node->parent = nullptr;
			}
			return;
		}

		for (Node* neighbour : currentNode->connections)
		{
			float gNew, hNew, fNew;
			if (IsInVector(closedList, neighbour) || neighbour->defencePlaced || !neighbour->reachable)
			{
				continue;
			}

			gNew = currentNode->g + sm::Vector3::Distance(neighbour->position, currentNode->position);
			hNew = sm::Vector3::Distance(goalNode->position, neighbour->position);
			fNew = gNew + hNew;

			if (gNew < currentNode->g || !IsInVector(openList, neighbour))
			{
				neighbour->g = gNew;
				neighbour->h = hNew;
				neighbour->f = fNew;

				neighbour->parent = currentNode;

				openList.emplace_back(neighbour);
			}
		}
	}

}

bool PathFinderManager::PlayerAStar(sm::Vector3 playerPos)
{
	std::vector<Node*> openList, closedList;
	Node* startingNode = FindClosestNode(playerPos);
	startingNode->g = 0.0f;
	startingNode->h = 0.0f;
	startingNode->f = 0.0f;
	openList.push_back(startingNode);
	Node* goalNode = m_nodes[0][0].get();
	//Node* goalNode = GetDistantNode(playerPos);
	while (!openList.empty())
	{
		Node* currentNode = openList.at(0);
		int ind = 0;
		for (int i = 1; i < openList.size(); i++)
		{
			if (openList[i]->f < currentNode->f)
			{
				currentNode = openList[i];
				ind = i;
			}
		}
		openList.erase(openList.begin() + ind);
		closedList.emplace_back(currentNode);

		if (currentNode == goalNode)
		{
			for (Node* node : openList)
			{
				node->ResetFGH();
				node->parent = nullptr;
			}
			for (Node* node : closedList)
			{
				node->ResetFGH();
				node->parent = nullptr;
			}
			return true;
		}

		for (Node* neighbour : currentNode->connections)
		{
			float gNew, hNew, fNew;
			if (IsInVector(closedList, neighbour) || neighbour->defencePlaced || !neighbour->reachable)
			{
				continue;
			}

			gNew = currentNode->g + sm::Vector3::Distance(neighbour->position, currentNode->position);
			hNew = sm::Vector3::Distance(goalNode->position, neighbour->position);
			fNew = gNew + hNew;

			if (gNew < currentNode->g || !IsInVector(openList, neighbour))
			{
				neighbour->g = gNew;
				neighbour->h = hNew;
				neighbour->f = fNew;

				neighbour->parent = currentNode;

				openList.emplace_back(neighbour);
			}
		}
	}
	for (Node* node : openList)
	{
		node->ResetFGH();
		node->parent = nullptr;
	}
	for (Node* node : closedList)
	{
		node->ResetFGH();
		node->parent = nullptr;
	}
	return false;
}

bool PathFinderManager::ReachedNode(const Entity npc)
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