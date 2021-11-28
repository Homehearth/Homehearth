#include "EnginePCH.h"
#include "GridSystem.h"
#include "PathFinderManager.h"


#define STB_IMAGE_IMPLEMENTATION

GridSystem::GridSystem()
{
	m_scene = nullptr;
	m_tileHalfWidth = 0.f;
}

GridSystem::~GridSystem()
{
}

bool GridSystem::InsideGrid(const int& xpos, const int& zpos) const
{
	return xpos >= 0 && m_gridSize.x && xpos < m_gridSize.x&& zpos >= 0 && m_gridSize.y && zpos < m_gridSize.y;
}

int GridSystem::TileOffset(const int& index) const
{
	bool isNegative = false;
	if (index % 2 == 0)
		isNegative = true;

	int offset = static_cast<int>(std::ceil(index / 2.0f));
	if (isNegative)
		offset *= -1;

	return offset;
}

void GridSystem::Initialize(Vector2I mapSize, sm::Vector3 position, std::string fileName, HeadlessScene* scene)
{
	m_scene = scene;
	m_position = position;
	m_mapSize = mapSize;

	std::string m_filepath = TEXTUREPATH + fileName;
	int width = 0;
	int height = 0;
	int comp = 0;

	unsigned char* pixelsData = stbi_load(m_filepath.c_str(), &width, &height, &comp, STBI_rgb_alpha); //TODO: find out what to do with this

	std::vector<int> pixelValues;
	m_gridSize = { width, height };
	m_tileSize = { (float)m_mapSize.x / m_gridSize.x, (float)m_mapSize.y / m_gridSize.y };

	for (int i = 0; i < m_gridSize.x * m_gridSize.y * 4; i++)
	{
		pixelValues.push_back((int)pixelsData[i]);
	}

	std::vector<Tile> rowTilesTemp;
	for (int col = 0; col < m_gridSize.y; col++)
	{
		for (int row = 0; row < m_gridSize.x; row++)
		{
			TileType tileTypeTemp = TileType::DEFAULT;

			sm::Vector4 rgba;

			//Colors from texture
			rgba.x = (float)pixelValues.at(0 + ((row + (col * (size_t)m_gridSize.y)) * 4));
			rgba.y = (float)pixelValues.at(1 + ((row + (col * (size_t)m_gridSize.y)) * 4));
			rgba.z = (float)pixelValues.at(2 + ((row + (col * (size_t)m_gridSize.y)) * 4));
			rgba.w = (float)pixelValues.at(3 + ((row + (col * (size_t)m_gridSize.y)) * 4));

			if (rgba == sm::Vector4{ 100, 100, 100, 255 })
				tileTypeTemp = TileType::EMPTY;

			if (rgba == sm::Vector4{ 255, 51, 0, 255 })
				tileTypeTemp = TileType::BUILDING;

			if (rgba == sm::Vector4{ 255, 234, 0, 255 } || rgba == sm::Vector4{ 0, 21, 255, 255 })
				tileTypeTemp = TileType::UNPLACABLE;

			if (rgba == sm::Vector4{ 0, 0, 0, 255 })
				tileTypeTemp = TileType::DEFAULT;

			m_tileHalfWidth = (m_tileSize.x / 2.f);
			sm::Vector3 tilePosition = { m_tileSize.x * row + m_tileHalfWidth, 0.f , (m_tileSize.y * -col) - m_tileHalfWidth };

			Tile tileTemp;
			tileTemp.gridID		= { col, row };
			tileTemp.type		= tileTypeTemp;
			tileTemp.position	= tilePosition;

			if (rowTilesTemp.size() < m_gridSize.x)
				rowTilesTemp.push_back(tileTemp);

			if (rowTilesTemp.size() >= m_gridSize.x)
				m_tiles.push_back(rowTilesTemp);

#if RENDER_GRID
			if (tileTemp.type == TileType::DEFAULT || tileTemp.type == TileType::EMPTY)
			{
				Entity tileEntity = m_scene->CreateEntity();
				comp::Transform* transform = tileEntity.AddComponent<comp::Transform>();
				tileEntity.AddComponent<comp::Network>();

				transform->position = tileTemp.position;

				tileEntity.AddComponent<comp::MeshName>()->name = "Cube.obj";
			}

			//transform->scale = { 4.2f, 0.5f, 4.2f };

			//if (tileTemp.type == TileType::EMPTY)
			//{
			//	tileEntity.AddComponent<comp::MeshName>()->name = "Plane1.obj";
			//}
			//else if (tileTemp.type == TileType::BUILDING || tileTemp.type == TileType::UNPLACABLE)
			//{

			//	tileEntity.AddComponent<comp::MeshName>()->name = "Plane2.obj";
			//}
			//else if (tileTemp.type == TileType::DEFAULT)
			//{
			//	tileEntity.AddComponent<comp::MeshName>()->name = "Plane3.obj";

			//}

#endif // RENDER_GRID


		}
		rowTilesTemp.clear();
	}
	stbi_image_free(pixelsData);
}

bool GridSystem::RemoveDefence(Ray_t& mouseRay, uint32_t playerWhoPressedMouse, PathFinderManager* aiHandler)
{
	sm::Vector3 localPlayer;
	m_scene->ForEachComponent<comp::Player, comp::Transform, comp::Network>([&](comp::Player& p, comp::Transform& t, comp::Network& net)
		{
			if (net.id == playerWhoPressedMouse)
			{
				localPlayer = t.position;
			}
		});

	float t = 0;
	float tMin = FLT_MAX;
	Entity closestEntity;
	m_scene->ForEachComponent<comp::Transform, comp::Tag<TagType::DEFENCE>, comp::OrientedBoxCollider>([&](Entity e, comp::Transform& transform, comp::Tag<TagType::DEFENCE>& d, comp::OrientedBoxCollider& b)
		{
			if (mouseRay.Intersects(b, &t))
			{
				if (t < tMin)
				{
					tMin = t;
					closestEntity = e;
				}
			}
		});
	if (tMin != FLT_MAX)
	{
		comp::TileSet* tileset = closestEntity.GetComponent<comp::TileSet>();
		if (tileset)
		{
			//	Go throgh all the tiles and fix them
			for (size_t i = 0; i < tileset->coordinates.size(); i++)
			{
				int zpos = tileset->coordinates[i].first;
				int xpos = tileset->coordinates[i].second;
				m_tiles[zpos][xpos].type = TileType::EMPTY;

				Node* node = aiHandler->GetNodeByID(m_tiles[zpos][xpos].gridID);
				node->defencePlaced = false;
				node->reachable = true;
				std::vector<Node*> diagNeighbors = node->GetDiagonalConnections();
				for (Node* diagNeighbor : diagNeighbors)
				{
					if (diagNeighbor->defencePlaced || !diagNeighbor->reachable)
					{
						Vector2I difference = node->id - diagNeighbor->id;
						Node* node1 = aiHandler->GetNodeByID(Vector2I(diagNeighbor->id.x + difference.x, diagNeighbor->id.y));
						Node* node2 = aiHandler->GetNodeByID(Vector2I(diagNeighbor->id.x, diagNeighbor->id.y + difference.y));

						node1->connections.push_back(node2);
						node2->connections.push_back(node1);
					}
				}
				if (aiHandler->PlayerAStar(localPlayer))
				{
					m_scene->ForEachComponent<comp::Player, comp::Network>([&](comp::Player& p, comp::Network& net)
						{
							if (net.id == playerWhoPressedMouse)
							{
								p.reachable = true;
							}
						});
				}
			}
		}

		Blackboard::Get().GetPathFindManager()->RemoveDefenseEntity(closestEntity);
		closestEntity.Destroy();
		
		return true;
	}
	else
	{
		return false;
	}
}

bool GridSystem::PlaceDefence(Ray_t& mouseRay, uint32_t playerWhoPressedMouse, PathFinderManager* aiHandler)
{	
	//Player that placed defence
	dx::BoundingSphere localPlayerSphere;
	comp::Player player;

	/*
		Save all the positions of the players and NPCs
	*/
	std::vector<dx::BoundingSphere> ePos;
	m_scene->ForEachComponent<comp::Player, comp::SphereCollider, comp::Network>([&](comp::Player& p, comp::SphereCollider bs, comp::Network& net)
		{
			if (net.id == playerWhoPressedMouse)
			{
				localPlayerSphere = bs;
				player = p;
			}
			ePos.push_back(bs);
		});
	m_scene->ForEachComponent<comp::NPC, comp::SphereCollider>([&](comp::NPC& p, comp::SphereCollider& bs)
		{
			ePos.push_back(bs);
		});

	Plane_t plane;
	plane.normal = { 0.0f, 1.0f, 0.0f };
	sm::Vector3 pos;
	if (mouseRay.Intersects(plane, &pos))
	{
		bool okayToPlace = true;

		//Tile touched
		int centerTileX = static_cast<int>(std::abs(pos.x) / m_tileSize.x);
		int centerTileZ = static_cast<int>(std::abs(pos.z) / m_tileSize.y);

		//Check distance between player and the tile what we touched
		float tileToPlayerDistance = (localPlayerSphere.Center - m_tiles[centerTileZ][centerTileX].position).Length();
		if (tileToPlayerDistance >= player.buildDistance)
			okayToPlace = false;

		UINT numberOfDefences = 0;
		if (player.towerSelected		== EDefenceType::SMALL)
			numberOfDefences = 1;
		else if (player.towerSelected	== EDefenceType::LARGE)
			numberOfDefences = 3;

		//All the coordinates of the tiles
		std::vector<std::pair<UINT, UINT>> coordinates;

		//Check if it was okay to place all the defences here
		for (UINT d = 0; d < numberOfDefences && okayToPlace; d++)
		{
			int xPos = centerTileX;
			int zPos = centerTileZ;
			if (player.rotateDefence)
				zPos += TileOffset(d);
			else
				xPos += TileOffset(d);
			
			coordinates.push_back({ zPos, xPos });
			Tile tile = m_tiles[zPos][xPos];

			//Check the tiles current type
			if (tile.type != TileType::EMPTY)
				okayToPlace = false;

			//Make sure we are within the grid
			if (InsideGrid(xPos, zPos))
			{
				//Each side of the tile
				float right		= tile.position.x + m_tileHalfWidth;
				float left		= tile.position.x - m_tileHalfWidth;
				float top		= tile.position.z + m_tileHalfWidth;
				float bottom	= tile.position.z - m_tileHalfWidth;

				// Checking so the other entities doesnt occupy the tile
				for (int i = 0; i < ePos.size() && okayToPlace; i++)
				{
					float closestX = max(left, min(ePos[i].Center.x, right));
					float closestZ = max(bottom, min(ePos[i].Center.z, top));

					//Distance between the tiles closest side and the entity
					float distance = (ePos[i].Center - sm::Vector3(closestX, 0.f, closestZ)).Length();

					//To close
					if (distance < ePos[i].Radius)
						okayToPlace = false;
				}
			}
		}

		// Okay to place defence here - JUST DO IT!
		if (okayToPlace)
		{
			//Go through all the tiles
			for (size_t t = 0; t < coordinates.size(); t++)
			{
				int zPos = coordinates[t].first;
				int xPos = coordinates[t].second;
				m_tiles[zPos][xPos].type = TileType::DEFENCE;

				Node* node = aiHandler->GetNodeByID(Vector2I(zPos, xPos));
				node->defencePlaced = true;
				node->reachable = false;

				//Check if connections need to be severed
				std::vector<Node*> diagNeighbors = node->GetDiagonalConnections();
				for (Node* diagNeighbor : diagNeighbors)
				{
					if (diagNeighbor->defencePlaced || !diagNeighbor->reachable)
					{
						Vector2I difference = node->id - diagNeighbor->id;
						Node* connectionRemovalNode1 = aiHandler->GetNodeByID(Vector2I(diagNeighbor->id.x + difference.x, diagNeighbor->id.y));
						Node* connectionRemovalNode2 = aiHandler->GetNodeByID(Vector2I(diagNeighbor->id.x, diagNeighbor->id.y + difference.y));
						if (!connectionRemovalNode1->RemoveConnection(connectionRemovalNode2))
						{
							LOG_INFO("Failed to remove connection1");
						}
						if (!connectionRemovalNode2->RemoveConnection(connectionRemovalNode1))
						{
							LOG_INFO("Failed to remove connection2");
						}
					}
				}
				if (!aiHandler->PlayerAStar(localPlayerSphere.Center))
				{
					m_scene->ForEachComponent<comp::Player, comp::Network>([&](comp::Player& p, comp::Network& net)
						{
							if (net.id == playerWhoPressedMouse)
							{
								p.reachable = false;
							}
						});
				}
			}

			/*
				Create the model for this tiles
			*/
			Tile centerTile = m_tiles[centerTileZ][centerTileX];
			Entity tileEntity = m_scene->CreateEntity();
			tileEntity.AddComponent<comp::Tag<TagType::STATIC>>();
			tileEntity.AddComponent<comp::Tag<TagType::DEFENCE>>();
			tileEntity.AddComponent<comp::Network>();
			coordinates.shrink_to_fit();
			tileEntity.AddComponent<comp::TileSet>()->coordinates = coordinates;
			
			comp::Transform*			transform	= tileEntity.AddComponent<comp::Transform>();
			comp::BoundingOrientedBox*	collider	= tileEntity.AddComponent<comp::BoundingOrientedBox>();
			comp::Health*				health		= tileEntity.AddComponent<comp::Health>();
			transform->position = { centerTile.position.x, 5.f, centerTile.position.z };

			if (player.rotateDefence)
			{
				transform->rotation = sm::Quaternion::CreateFromYawPitchRoll(static_cast<float>(PI / 2.f), 0.f, 0.f);
				collider->Extents = { m_tileHalfWidth, m_tileHalfWidth, m_tileHalfWidth * numberOfDefences };
			}
			else
			{ 
				collider->Extents = { m_tileHalfWidth * numberOfDefences, m_tileHalfWidth, m_tileHalfWidth };
			}
			
			if (player.towerSelected		== EDefenceType::SMALL)
			{
				health->currentHealth	= 100.0f;
				health->maxHealth		= 100.0f;
				tileEntity.AddComponent<comp::MeshName>()->name = NameType::MESH_DEFENCE1X1;
			}
			else if (player.towerSelected	== EDefenceType::LARGE)
			{
				health->currentHealth	= 300.0f;
				health->maxHealth		= 300.0f;
				tileEntity.AddComponent<comp::MeshName>()->name = NameType::MESH_DEFENCE1X3;
			}

			return true;
		}
	}
	return false;
}

uint32_t GridSystem::GetTileCount() const
{
	return m_gridSize.x * m_gridSize.y;
}

Vector2I GridSystem::GetGridSize() const
{
	return m_gridSize;
}


Tile* GridSystem::GetTile(Vector2I& id)
{
	return &m_tiles.at(id.x).at(id.y);
}

std::vector<Entity>* GridSystem::GetTileEntities()
{
	return nullptr;
}
