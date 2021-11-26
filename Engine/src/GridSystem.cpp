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
			tileTemp.gridID = { col, row };
			tileTemp.halfWidth = m_tileHalfWidth;
			tileTemp.type = tileTypeTemp;
			tileTemp.position = tilePosition;

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
		comp::Transform* closestTransform = closestEntity.GetComponent<comp::Transform>();
		int clampedX = static_cast<int>((abs(closestTransform->position.x) / m_tileSize.x));
		int clampedZ = static_cast<int>((abs(closestTransform->position.z) / m_tileSize.y));

		m_tiles[clampedZ][clampedX].type = TileType::EMPTY;
		Node* node = aiHandler->GetNodeByID(m_tiles[clampedZ][clampedX].gridID);
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
	Plane_t plane;
	plane.normal = { 0.0f, 1.0f, 0.0f };
	sm::Vector3 pos;
	bool canBuild = false;

	dx::BoundingSphere localPlayer;
	std::vector<dx::BoundingSphere> ePos;

	// Save positions to calculate distances to the tile for players
	m_scene->ForEachComponent<comp::Player, comp::SphereCollider, comp::Network>([&](comp::Player& p, comp::SphereCollider bs, comp::Network& net)
		{
			if (net.id == playerWhoPressedMouse)
			{
				localPlayer = bs;
			}
			ePos.push_back(bs);
		});
	// Do the same for all NPC entities
	m_scene->ForEachComponent<comp::NPC, comp::SphereCollider>([&](comp::NPC& p, comp::SphereCollider& bs)
		{
			ePos.push_back(bs);
		});

	if (mouseRay.Intersects(plane, &pos))
	{

		int clampedX = static_cast<int>((abs(pos.x) / m_tileSize.x));
		int clampedZ = static_cast<int>((abs(pos.z) / m_tileSize.y));
		Tile tile = m_tiles[clampedZ][clampedX];

		// Basically an AABB check
		float right = tile.position.x + tile.halfWidth;
		float left = tile.position.x - tile.halfWidth;
		float top = tile.position.z + tile.halfWidth;
		float bottom = tile.position.z - tile.halfWidth;

		// Is mouse position we clicked at within the tiles bounds
		if (pos.x > left && pos.x < right && pos.z < top && pos.z > bottom)
		{
			if (tile.type == TileType::EMPTY)
			{
				bool tileOccupied = false;
				// Checking so the other entities doesnt occupy the tile
				for (int i = 0; i < ePos.size() && !tileOccupied; i++)
				{
					// Is the entity occupying a tile?

					float closestX = max(left, min(ePos[i].Center.x, right));
					float closestZ = max(bottom, min(ePos[i].Center.z, top));

					sm::Vector3 pointToSphere = ePos[i].Center - sm::Vector3(closestX, 0.f, closestZ);
					float distance = pointToSphere.Length();

					if (distance < ePos[i].Radius)
					{
						tileOccupied = true;
					}
				}
				if (!tileOccupied)
				{
					m_tiles[clampedZ][clampedX].type = TileType::DEFENCE;

					Entity defenseEntity = m_scene->CreateEntity();
					comp::Transform* transform = defenseEntity.AddComponent<comp::Transform>();
					transform->position = { tile.position.x , 5.f, tile.position.z };
					transform->scale = { 1.35f, 1.f, 1.35f };

					comp::OrientedBoxCollider* collider = defenseEntity.AddComponent<comp::OrientedBoxCollider>();
					collider->Extents = { m_tileHalfWidth, m_tileHalfWidth, m_tileHalfWidth };
					defenseEntity.AddComponent<comp::Tag<TagType::STATIC>>();
					defenseEntity.AddComponent<comp::Tag<TagType::DEFENCE>>();
					defenseEntity.AddComponent<comp::MeshName>()->name = NameType::MESH_DEFENCE;
					defenseEntity.AddComponent<comp::Health>();
					defenseEntity.AddComponent<comp::Network>();
					aiHandler->AddDefenseEntity(defenseEntity);

					Node* node = aiHandler->GetNodeByID(Vector2I(clampedZ, clampedX));
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
					if (!aiHandler->PlayerAStar(localPlayer.Center))
					{
						m_scene->ForEachComponent<comp::Player, comp::Network>([&](comp::Player& p, comp::Network& net)
							{
								if (net.id == playerWhoPressedMouse)
								{
									p.reachable = false;
								}
							});
					}
					return true;
				}
			}
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
