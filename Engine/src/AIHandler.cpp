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


	return currentClosest;
}

std::vector<Node*> AIHandler::GetNeighbors(GridSystem* grid, Tile* baseNode)
{
	std::vector<Node*> neighbors;
	if ((baseNode->type == TileType::DEFAULT
		|| baseNode->type == TileType::EMPTY))
	{

		Tile* currentTile = nullptr;
		for (int i = max(0, baseNode->gridID.x - 1); i <= min(baseNode->gridID.x + 1, grid->GetGridSize().x - 1); i++)
		{
			for (int j = max(0, baseNode->gridID.y - 1); j <= min(baseNode->gridID.y + 1, grid->GetGridSize().y - 1); j++)
			{
				if (i != baseNode->gridID.x || j != baseNode->gridID.y)
				{
					currentTile = grid->GetTile(Vector2I(i, j));
					if (currentTile->type == TileType::DEFAULT
						|| currentTile->type == TileType::EMPTY)
					{
						neighbors.push_back(GetNodeByID({ i,j }));
					}
				}
			}
		}


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
			if (grid->GetTile(Vector2I(i, j))->type == TileType::BUILDING ||
				grid->GetTile(Vector2I(i, j))->type == TileType::DEFENCE ||
				grid->GetTile(Vector2I(i, j))->type == TileType::UNPLACABLE)
			{
				currentNode->reachable = false;
			}
			AddNode(currentNode);
		}
	}

	//Build Connections
	LOG_INFO("AIHandler: Connecting Nodes");
	for (int i = 0; i < grid->GetGridSize().x; i++)
	{
		for (int j = 0; j < grid->GetGridSize().y; j++)
		{
			Tile* entityTile = grid->GetTile(Vector2I(i, j));

			std::vector<Node*> neighbors = GetNeighbors(grid, entityTile);
			for (auto neighbor : neighbors)
			{
				m_nodes[i][j]->connections.push_back(neighbor);
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

void AIHandler::AStarSearch(HeadlessScene& scene, Entity npc)
{
	comp::NPC* npcComp = npc.GetComponent<comp::NPC>();
	comp::Transform* npcTransform = npc.GetComponent<comp::Transform>();
	Node* currentNode = npcComp->currentNode;

	Entity closestPlayer = FindClosestPlayer(scene, npcTransform->position, npcComp);
	comp::Transform* playerTransform = closestPlayer.GetComponent<comp::Transform>();

	std::vector<Node*> closedList, openList;
	npcComp->path.clear();
	Node* startingNode = currentNode, * goalNode = FindClosestNode(playerTransform->position);;
	openList.push_back(startingNode);
	startingNode->f = 0.f;
	startingNode->g = 0.f;
	startingNode->h = 0.f;
	startingNode->parent = startingNode;

	Node* nodeToAdd = nullptr;
	while (!openList.empty() && nodeToAdd != goalNode)
	{
		nodeToAdd = openList.at(0);
		int indexToPop = 0;
		bool stop = false;
		for (unsigned int i = 0; i < openList.size(); i++)
		{
			if (openList.at(i)->f < nodeToAdd->f)
			{
				nodeToAdd = openList.at(i);
				indexToPop = i;
			}
		}
		openList.erase(openList.begin() + indexToPop);

		//Neighbors

		std::vector<Node*> neighbors = nodeToAdd->connections;

		for (Node* neighbor : neighbors)
		{
			if (neighbor->parent != nodeToAdd && neighbor != nodeToAdd)
			{
				if (!neighbor->parent)
				{
					neighbor->parent = nodeToAdd;
				}
				if (neighbor == goalNode)
				{
					nodeToAdd = goalNode;
					break;
				}
				if (neighbor->f == FLT_MAX)
				{
					float tempF = 0, tempG = 0, tempH = 0;

					tempG = nodeToAdd->g + (nodeToAdd->position - neighbor->position).Length();
					tempH = (goalNode->position - nodeToAdd->position).Length(); //Using euclidean distance
					tempF = tempG + tempH;
					neighbor->f = tempF;
					neighbor->g = tempG;
					neighbor->h = tempH;
				}
				stop = false;
				for (unsigned int i = 0; i < openList.size() && !stop; i++)
				{
					if (openList.at(i)->id == neighbor->id)
					{
						stop = true;
					}
				}
				if (closedList.size() > 0)
				{
					for (unsigned int i = 0; i < closedList.size() && !stop; i++)
					{
						if (closedList.at(i)->id == neighbor->id)
						{
							stop = true;
						}
					}
					if (!stop)
					{
						openList.push_back(neighbor);
					}
				}
				else
				{
					openList.push_back(neighbor);
				}
			}
		}

		closedList.push_back(nodeToAdd);
	}

	//TracePath
	if (goalNode)
		currentNode = goalNode;

	while (currentNode != startingNode)
	{
		//Insert currentNode to the path
		npcComp->path.insert(npcComp->path.begin(), currentNode);
		currentNode = currentNode->parent;
	}


	//scene.ForEachComponent<Node>([&](Entity entity, Node& node)
	//	{
	//		node.ResetFGH();
	//		node.parent = nullptr;
	//	});

}
