#include "EnginePCH.h"
#include "AIHandler.h"

std::vector<std::unique_ptr<AIHandler::Node>>& AIHandler::GetNeighbors(std::unique_ptr<AIHandler::Node>& node, comp::Tile& tile, const Vector2I& gridSize)
{
	std::vector<std::unique_ptr<Node>> neighbors;

}

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

AIHandler::Node* AIHandler::FindClosestNode(sm::Vector3 position)
{
	Node* currentClosest = nullptr;


	return currentClosest;
}

AIHandler::Node* AIHandler::GetNodeByID(Vector2I id) const
{
	try
	{
		return m_nodes.at(id).get();
	}
	catch (std::out_of_range)
	{
		return nullptr;
	}
}

bool AIHandler::AddNode(Vector2I id)
{
	if (!GetNodeByID(id))
	{
		return false;
	}
	else
	{
		m_nodes.insert({ id, std::make_unique<Node>(id) });
		return true;
	}
}

bool AIHandler::AddNode(std::unique_ptr<Node>& node)
{
	if (!GetNodeByID(node->id))
	{
		return false;
	}
	else
	{
		m_nodes.insert({ node->id, std::move(node) });
		return true;
	}
}

void AIHandler::CreateNodes(GridSystem* grid, HeadlessScene* scene)
{
	int itrID = 0;
	std::vector<Entity>* tiles = grid->GetTiles();
	//Create Nodes
	for (int i = 0; i < tiles->size(); i++)
	{
		std::unique_ptr<Node> currentNode = std::make_unique<Node>(tiles->at(i).GetComponent<comp::Tile>()->gridID);
		if (tiles->at(i).GetComponent<comp::Tile>()->type == TileType::BUILDING ||
			tiles->at(i).GetComponent<comp::Tile>()->type == TileType::DEFENCE ||
			tiles->at(i).GetComponent<comp::Tile>()->type == TileType::UNPLACABLE)
		{
			currentNode->reachable = false;
		}
		AddNode(currentNode);
	}

	//Build Connections
	for (int i = 0; i < tiles->size(); i++)
	{
		comp::Tile* entityTile = tiles->at(i).GetComponent<comp::Tile>();
		Vector2I currentID = entityTile->gridID;
		//Get Neighbors
		Entity* currentTile = grid->GetTileByID(currentID);
		if ((currentTile->GetComponent<comp::Tile>()->type == TileType::DEFAULT
			|| currentTile->GetComponent<comp::Tile>()->type == TileType::EMPTY))
		{
			Vector2I gridSize = grid->GetGridSize();
			currentTile = grid->GetTileByID(currentID + sm::Vector2(-1, 0));
			if (gridSize.x > 0)
			{
				for (int i = max(0, currentID.x - 1); i <= min(currentID.x + 1, gridSize.x); i++)
				{
					for (int j = max(0, currentID.y - 1); j <= min(currentID.y + 1, gridSize.y); j++)
					{
						if (i != currentID.x || j != currentID.y)
						{
							currentTile = grid->GetTileByID({ i,j });
							if (currentTile->GetComponent<comp::Tile>()->type == TileType::DEFAULT
								|| currentTile->GetComponent<comp::Tile>()->type == TileType::EMPTY)
							{
								m_nodes.at({ i,j })->connections.push_back(GetNodeByID({ i,j }));
							}
						}
					}
				}
			}

			//LOG_INFO("Connections: %d", nodes.at(i)->connections.size());
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

		std::vector<comp::Node*> neighbors = nodeToAdd->connections;

		for (comp::Node* neighbor : neighbors)
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

	scene.ForEachComponent<comp::Node>([&](Entity entity, comp::Node& node)
		{
			node.ResetFGH();
			node.parent = nullptr;
		});

	return true;
}
