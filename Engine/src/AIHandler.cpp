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

	for (int i = 0; i < m_nodes.size(); i++)
	{
		for (int j = 0; j < m_nodes[i].size(); j++)
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

		for (int newY = -1; newY <= 1; newY++)
		{
			for (int newX = -1; newX <= 1; newX++)
			{
				if (baseNode->gridID.x + newX >= grid->GetGridSize().x || baseNode->gridID.y + newY >= grid->GetGridSize().y
					|| baseNode->gridID.x + newX < 0 || baseNode->gridID.y + newY < 0)
					continue;

				if(baseNode->gridID.x + newX == 38 && baseNode->gridID.y + newY == 20)
				{

				}
				currentTile = grid->GetTile(Vector2I(baseNode->gridID.x + newX, baseNode->gridID.y + newY));

				if (currentTile->type == TileType::DEFAULT || currentTile->type == TileType::EMPTY)
				{
					neighbors.push_back(GetNodeByID({ baseNode->gridID.x + newX,baseNode->gridID.y + newY }));
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

bool AIHandler::AddNode(Vector2I id)
{
	if (GetNodeByID(id))
	{
		return false;
	}
	else
	{
		m_nodes[id.x][id.y] = std::make_unique<Node>(id);
		return true;
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

	for (int i = 0; i < grid->GetGridSize().x; i++)
	{
		for (int j = 0; j < grid->GetGridSize().y; j++)
		{
			Vector2I tileID = { i,j };
			Vector2I nodeID = grid->GetTile(tileID)->gridID;
			std::unique_ptr<Node> currentNode = std::make_unique<Node>(nodeID);
			currentNode->position = grid->GetTile(Vector2I(i, j))->position;

			if (grid->GetTile(Vector2I(i, j))->type == TileType::DEFENCE ||
				grid->GetTile(Vector2I(i, j))->type == TileType::UNPLACABLE ||
				grid->GetTile(Vector2I(i, j))->type == TileType::BUILDING)
			{
				currentNode->reachable = false;
			}
			AddNode(currentNode);
		}
	}
	//Build Connections
	LOG_INFO("AIHandler: Connecting Nodes");
	for (int y = 0; y < grid->GetGridSize().y; y++)
	{
		for (int x = 0; x < grid->GetGridSize().x; x++)
		{
			Tile* entityTile = grid->GetTile(Vector2I(x, y));

			std::vector<Node*> neighbors = GetNeighbors(grid, entityTile);
			for (auto neighbor : neighbors)
			{
				if(neighbor->reachable)
					m_nodes[y][x]->connections.push_back(neighbor);
			}
		}
	}

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
	
	std::vector<Node*> openList, closedList;
	comp::NPC* npcComp = npc.GetComponent<comp::NPC>();

	while (!npcComp->path.empty())
		npcComp->path.pop();

	comp::Transform* npcTransform = npc.GetComponent<comp::Transform>();

	if(npcComp->currentNode == nullptr)
	{
		this->SetClosestNode(*npcComp, npcTransform->position);
	}

	openList.push_back(npcComp->currentNode);

	Entity closestPlayer = FindClosestPlayer(scene, npcTransform->position, npcComp);
	comp::Transform* playerTransform = closestPlayer.GetComponent<comp::Transform>();
	//Node* currentNode = npcComp->currentNode,
	Node* goalNode = FindClosestNode(playerTransform->position);
	Node* startingNode = npcComp->currentNode;
	while (!openList.empty())
	{
		int itrToRemove = 0;
		Node* currentNode = openList[0];
		for (int i = 0; i < openList.size(); i++)
		{
			if (openList.at(i)->f < currentNode->f)
			{
				currentNode = openList[i];
				itrToRemove = i;
			}
		}

		openList.erase(openList.begin() + itrToRemove);
		closedList.push_back(currentNode);



		if (currentNode == goalNode)
		{
			//Trace the path back
			if (goalNode->parent)
			{
				while (currentNode != startingNode)
				{
					npcComp->path.emplace(currentNode);
					currentNode = currentNode->parent;
				}
			}
			break;
		}

		for (Node* neighbor : currentNode->connections)
		{
			if (!IsInVector(closedList, neighbor))
			{
				neighbor->g = currentNode->g + 1.0f;
				neighbor->h = (neighbor->position - goalNode->position).Length(); //Using euclidean distance
				neighbor->f = neighbor->g + neighbor->h;

				bool inOpen = false;

				for (int i = 0; i < openList.size() && !inOpen; i++)
				{
					if (neighbor == openList[i])
					{
						inOpen = true;
					}
				}

				if (!inOpen)
				{
					neighbor->parent = currentNode;
					openList.emplace_back(neighbor);
				}

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
