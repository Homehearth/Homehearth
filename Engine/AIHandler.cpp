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
	//int itrID = 0;
	//std::vector<Entity>* tiles = grid->GetTiles();
	////Create Nodes
	//for (int i = 0; i < tiles->size(); i++)
	//{
	//	std::unique_ptr<Node> currentNode = std::make_unique<Node>(tiles->at(i).GetComponent<comp::Tile>()->gridID);
	//	if (tiles->at(i).GetComponent<comp::Tile>()->type == TileType::BUILDING ||
	//		tiles->at(i).GetComponent<comp::Tile>()->type == TileType::DEFENCE ||
	//		tiles->at(i).GetComponent<comp::Tile>()->type == TileType::UNPLACABLE)
	//	{
	//		currentNode->reachable = false;
	//	}
	//	AddNode(currentNode);
	//}

	////Build Connections
	//for (int i = 0; i < tiles->size(); i++)
	//{
	//	comp::Tile* entityTile = tiles->at(i).GetComponent<comp::Tile>();
	//	Vector2I currentID = entityTile->gridID;
	//	//Get Neighbors
	//	Entity* currentTile = grid->GetTileByID(currentID);
	//	if ((currentTile->GetComponent<comp::Tile>()->type == TileType::DEFAULT
	//		|| currentTile->GetComponent<comp::Tile>()->type == TileType::EMPTY))
	//	{
	//		currentTile = grid->GetTileByID(currentID + sm::Vector2(-1, 0));
	//		//TODO: Improve this bad code. EXTREMELY TEMPORARY
	//		//Left
	//		if (currentTile && (currentTile->GetComponent<comp::Tile>()->type == TileType::DEFAULT
	//			|| currentTile->GetComponent<comp::Tile>()->type == TileType::EMPTY))
	//		{
	//			if (!nodes.at(i)->ConnectionAlreadyExists(GetNodeByID(currentID + sm::Vector2(-1, 0))))
	//			{
	//				nodes.at(i)->connections.push_back(GetAINodeById(currentID + sm::Vector2(-1, 0)));
	//			}
	//		}
	//		currentTile = m_grid.GetTileByID(currentID + sm::Vector2(-1, 1));
	//		//Up-left
	//		if (currentTile && (currentTile->GetComponent<comp::Tile>()->type == TileType::DEFAULT
	//			|| currentTile->GetComponent<comp::Tile>()->type == TileType::EMPTY))
	//		{
	//			if (!nodes.at(i)->ConnectionAlreadyExists(GetAINodeById(currentID + sm::Vector2(-1, 1))))
	//			{
	//				nodes.at(i)->connections.push_back(GetAINodeById(currentID + sm::Vector2(-1, 1)));
	//			}
	//		}
	//		currentTile = m_grid.GetTileByID(currentID + sm::Vector2(1, 0));
	//		//Right
	//		if (currentTile && (currentTile->GetComponent<comp::Tile>()->type == TileType::DEFAULT
	//			|| currentTile->GetComponent<comp::Tile>()->type == TileType::EMPTY))
	//		{
	//			if (!nodes.at(i)->ConnectionAlreadyExists(GetAINodeById(currentID + sm::Vector2(1, 0))))
	//			{
	//				nodes.at(i)->connections.push_back(GetAINodeById(currentID + sm::Vector2(1, 0)));
	//			}
	//		}
	//		currentTile = m_grid.GetTileByID(currentID + sm::Vector2(1, 1));
	//		//Up-right
	//		if (currentTile && (currentTile->GetComponent<comp::Tile>()->type == TileType::DEFAULT
	//			|| currentTile->GetComponent<comp::Tile>()->type == TileType::EMPTY))
	//		{
	//			if (!nodes.at(i)->ConnectionAlreadyExists(GetAINodeById(currentID + sm::Vector2(1, 1))))
	//			{
	//				nodes.at(i)->connections.push_back(GetAINodeById(currentID + sm::Vector2(1, 1)));
	//			}
	//		}
	//		currentTile = m_grid.GetTileByID(currentID + sm::Vector2(0, 1));
	//		//Up
	//		if (currentTile && (currentTile->GetComponent<comp::Tile>()->type == TileType::DEFAULT
	//			|| currentTile->GetComponent<comp::Tile>()->type == TileType::EMPTY))
	//		{
	//			if (!nodes.at(i)->ConnectionAlreadyExists(GetAINodeById(currentID + sm::Vector2(0, 1))))
	//			{
	//				nodes.at(i)->connections.push_back(GetAINodeById(currentID + sm::Vector2(0, 1)));
	//			}
	//		}
	//		currentTile = m_grid.GetTileByID(currentID + sm::Vector2(-1, -1));
	//		//Down-left
	//		if (currentTile && (currentTile->GetComponent<comp::Tile>()->type == TileType::DEFAULT
	//			|| currentTile->GetComponent<comp::Tile>()->type == TileType::EMPTY))
	//		{
	//			if (!nodes.at(i)->ConnectionAlreadyExists(GetAINodeById(currentID + sm::Vector2(-1, -1))))
	//			{
	//				nodes.at(i)->connections.push_back(GetAINodeById(currentID + sm::Vector2(-1, -1)));
	//			}
	//		}
	//		currentTile = m_grid.GetTileByID(currentID + sm::Vector2(0, -1));
	//		//Down
	//		if (currentTile && (currentTile->GetComponent<comp::Tile>()->type == TileType::DEFAULT
	//			|| currentTile->GetComponent<comp::Tile>()->type == TileType::EMPTY))
	//		{
	//			if (!nodes.at(i)->ConnectionAlreadyExists(GetAINodeById(currentID + sm::Vector2(0, -1))))
	//			{
	//				nodes.at(i)->connections.push_back(GetAINodeById(currentID + sm::Vector2(0, -1)));
	//			}
	//		}
	//		currentTile = m_grid.GetTileByID(currentID + sm::Vector2(1, -1));
	//		//Down-right
	//		if (currentTile && (currentTile->GetComponent<comp::Tile>()->type == TileType::DEFAULT
	//			|| currentTile->GetComponent<comp::Tile>()->type == TileType::EMPTY))
	//		{
	//			if (!nodes.at(i)->ConnectionAlreadyExists(GetAINodeById(currentID + sm::Vector2(1, -1))))
	//			{
	//				nodes.at(i)->connections.push_back(GetAINodeById(currentID + sm::Vector2(1, -1)));
	//			}
	//		}
	//		//LOG_INFO("Connections: %d", nodes.at(i)->connections.size());
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

void AIHandler::AStarSearch(HeadlessScene& scene, Entity npc)
{
}
