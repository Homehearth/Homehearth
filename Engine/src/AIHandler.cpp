#include "EnginePCH.h"
#include "AIHandler.h"

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
			currentTile = grid->GetTileByID(currentID + sm::Vector2(-1, 0));
			//TODO: Improve this bad code. EXTREMELY TEMPORARY
			//Left
			if (currentTile && (currentTile->GetComponent<comp::Tile>()->type == TileType::DEFAULT
				|| currentTile->GetComponent<comp::Tile>()->type == TileType::EMPTY))
			{
				if (!nodes.at(i)->ConnectionAlreadyExists(GetNodeByID(currentID + sm::Vector2(-1, 0))))
				{
					nodes.at(i)->connections.push_back(GetAINodeById(currentID + sm::Vector2(-1, 0)));
				}
			}
			currentTile = m_grid.GetTileByID(currentID + sm::Vector2(-1, 1));
			//Up-left
			if (currentTile && (currentTile->GetComponent<comp::Tile>()->type == TileType::DEFAULT
				|| currentTile->GetComponent<comp::Tile>()->type == TileType::EMPTY))
			{
				if (!nodes.at(i)->ConnectionAlreadyExists(GetAINodeById(currentID + sm::Vector2(-1, 1))))
				{
					nodes.at(i)->connections.push_back(GetAINodeById(currentID + sm::Vector2(-1, 1)));
				}
			}
			currentTile = m_grid.GetTileByID(currentID + sm::Vector2(1, 0));
			//Right
			if (currentTile && (currentTile->GetComponent<comp::Tile>()->type == TileType::DEFAULT
				|| currentTile->GetComponent<comp::Tile>()->type == TileType::EMPTY))
			{
				if (!nodes.at(i)->ConnectionAlreadyExists(GetAINodeById(currentID + sm::Vector2(1, 0))))
				{
					nodes.at(i)->connections.push_back(GetAINodeById(currentID + sm::Vector2(1, 0)));
				}
			}
			currentTile = m_grid.GetTileByID(currentID + sm::Vector2(1, 1));
			//Up-right
			if (currentTile && (currentTile->GetComponent<comp::Tile>()->type == TileType::DEFAULT
				|| currentTile->GetComponent<comp::Tile>()->type == TileType::EMPTY))
			{
				if (!nodes.at(i)->ConnectionAlreadyExists(GetAINodeById(currentID + sm::Vector2(1, 1))))
				{
					nodes.at(i)->connections.push_back(GetAINodeById(currentID + sm::Vector2(1, 1)));
				}
			}
			currentTile = m_grid.GetTileByID(currentID + sm::Vector2(0, 1));
			//Up
			if (currentTile && (currentTile->GetComponent<comp::Tile>()->type == TileType::DEFAULT
				|| currentTile->GetComponent<comp::Tile>()->type == TileType::EMPTY))
			{
				if (!nodes.at(i)->ConnectionAlreadyExists(GetAINodeById(currentID + sm::Vector2(0, 1))))
				{
					nodes.at(i)->connections.push_back(GetAINodeById(currentID + sm::Vector2(0, 1)));
				}
			}
			currentTile = m_grid.GetTileByID(currentID + sm::Vector2(-1, -1));
			//Down-left
			if (currentTile && (currentTile->GetComponent<comp::Tile>()->type == TileType::DEFAULT
				|| currentTile->GetComponent<comp::Tile>()->type == TileType::EMPTY))
			{
				if (!nodes.at(i)->ConnectionAlreadyExists(GetAINodeById(currentID + sm::Vector2(-1, -1))))
				{
					nodes.at(i)->connections.push_back(GetAINodeById(currentID + sm::Vector2(-1, -1)));
				}
			}
			currentTile = m_grid.GetTileByID(currentID + sm::Vector2(0, -1));
			//Down
			if (currentTile && (currentTile->GetComponent<comp::Tile>()->type == TileType::DEFAULT
				|| currentTile->GetComponent<comp::Tile>()->type == TileType::EMPTY))
			{
				if (!nodes.at(i)->ConnectionAlreadyExists(GetAINodeById(currentID + sm::Vector2(0, -1))))
				{
					nodes.at(i)->connections.push_back(GetAINodeById(currentID + sm::Vector2(0, -1)));
				}
			}
			currentTile = m_grid.GetTileByID(currentID + sm::Vector2(1, -1));
			//Down-right
			if (currentTile && (currentTile->GetComponent<comp::Tile>()->type == TileType::DEFAULT
				|| currentTile->GetComponent<comp::Tile>()->type == TileType::EMPTY))
			{
				if (!nodes.at(i)->ConnectionAlreadyExists(GetAINodeById(currentID + sm::Vector2(1, -1))))
				{
					nodes.at(i)->connections.push_back(GetAINodeById(currentID + sm::Vector2(1, -1)));
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
	comp::Node* currentNode = npcComp->currentNode;

	Entity closestPlayer = FindClosestPlayer(scene, npcTransform->position, npcComp);
	comp::Transform* playerTransform = closestPlayer.GetComponent<comp::Transform>();

	std::vector<comp::Node*> closedList, openList;
	npcComp->path.clear();
	comp::Node* startingNode = currentNode, * goalNode = FindClosestNode(scene, playerTransform->position);;
	openList.push_back(startingNode);
	startingNode->f = 0.f;
	startingNode->g = 0.f;
	startingNode->h = 0.f;
	startingNode->parent = startingNode;

	comp::Node* nodeToAdd = nullptr;
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
