#include "EnginePCH.h"
#include "GridSystem.h"
#include "PathFinderManager.h"
#include "QuadTree.h"


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

sm::Vector3 GridSystem::CalcCenterPoint(const std::vector<std::pair<UINT, UINT>>& coordinates)
{
	sm::Vector3 center = { 0,0,0 };

	if (!coordinates.empty())
	{
		sm::Vector3 min = sm::Vector3(static_cast<float>(m_mapSize.x), m_tileHalfWidth * 2.f, static_cast<float>(m_mapSize.y));
		sm::Vector3 max = min * -1;

		for (size_t i = 0; i < coordinates.size(); i++)
		{
			UINT zPos = coordinates[i].first;
			UINT xPos = coordinates[i].second;
			Tile tile = m_tiles[zPos][xPos];
			sm::Vector3 minTile = tile.position - sm::Vector3(m_tileHalfWidth);
			sm::Vector3 maxTile = tile.position + sm::Vector3(m_tileHalfWidth);

			if (minTile.x < min.x)
				min.x = minTile.x;
			if (maxTile.x > max.x)
				max.x = maxTile.x;

			if (minTile.y < min.y)
				min.y = minTile.y;
			if (maxTile.y > max.y)
				max.y = maxTile.y;

			if (minTile.z < min.z)
				min.z = minTile.z;
			if (maxTile.z > max.z)
				max.z = maxTile.z;
		}

		center = (min + max) / 2.f;
	}
	return center;
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

				tileEntity.AddComponent<comp::MeshName>()->name = NameType::MESH_CUBE;
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

std::vector<Entity> GridSystem::UpdateHoverDefence()
{
	/*
		Not fully optimized but works for now
	*/
	std::vector<Entity> entities;

	//Go through the players
	m_scene->ForEachComponent<comp::Player, comp::Network>([&](comp::Player& player, comp::Network& net)
		{
			bool okayToPlace = false;

			//Did not find the hover defence for this player
			if (m_hoveredDefences.find(net.id) == m_hoveredDefences.end())
			{
				Entity entity1 = m_scene->CreateEntity();
				entity1.AddComponent<comp::Transform>();
				entity1.AddComponent<comp::Network>();
				entity1.AddComponent<comp::MeshName>()->name = NameType::MESH_DEFENCE1X1;

				Entity entity2 = m_scene->CreateEntity();
				entity2.AddComponent<comp::Transform>();
				entity2.AddComponent<comp::Network>();
				entity2.AddComponent<comp::MeshName>()->name = NameType::MESH_DEFENCE1X3;
				m_hoveredDefences[net.id] = { entity1, entity2 };
			}
			else
			{
				if (player.shopItem == ShopItem::Defence1x1 ||
					player.shopItem == ShopItem::Defence1x3)
				{
					if (!CheckDefenceLocation(player.inputState.mouseRay, net.id).empty())
						okayToPlace = true;
				}

				if (okayToPlace)
				{
					Plane_t plane;
					plane.normal = { 0.0f, 1.0f, 0.0f };
					sm::Vector3 pos;
					if (player.inputState.mouseRay.Intersects(plane, &pos))
					{
						//Snap the position to the grid
						int centerTileX = static_cast<int>(std::abs(pos.x) / m_tileSize.x);
						int centerTileZ = static_cast<int>(std::abs(pos.z) / m_tileSize.y);
						if (InsideGrid(centerTileX, centerTileZ))
							pos = m_tiles[centerTileZ][centerTileX].position;
						pos.y = 5.f;

						comp::Transform* transform;

						if (player.shopItem == ShopItem::Defence1x1)
						{
							transform = m_hoveredDefences.at(net.id).def1x1.GetComponent<comp::Transform>();
							transform->position = pos;
							if (player.rotateDefence)
								transform->rotation = sm::Quaternion::CreateFromYawPitchRoll(static_cast<float>(PI / 2.f), 0.f, 0.f);
							else
								transform->rotation = sm::Quaternion(0, 0, 0, 0);

							m_hoveredDefences.at(net.id).def1x1.GetComponent<comp::MeshName>()->name = NameType::MESH_DEFENCE1X1;

							entities.push_back(m_hoveredDefences.at(net.id).def1x1);
						}
						else if (player.shopItem == ShopItem::Defence1x3)
						{
							transform = m_hoveredDefences.at(net.id).def1x3.GetComponent<comp::Transform>();
							transform->position = pos;
							if (player.rotateDefence)
								transform->rotation = sm::Quaternion::CreateFromYawPitchRoll(static_cast<float>(PI / 2.f), 0.f, 0.f);
							else
								transform->rotation = sm::Quaternion(0, 0, 0, 0);

							m_hoveredDefences.at(net.id).def1x3.GetComponent<comp::MeshName>()->name = NameType::MESH_DEFENCE1X3;

							entities.push_back(m_hoveredDefences.at(net.id).def1x3);
						}
					}
				}
				else
				{
					comp::Transform* trans1 = m_hoveredDefences.at(net.id).def1x1.GetComponent<comp::Transform>();
					if (trans1)
					{
						if (trans1->position != sm::Vector3(0, 0, 0))
						{
							trans1->position = sm::Vector3(0, 0, 0);
							entities.push_back(m_hoveredDefences.at(net.id).def1x1);
						}
					}
					comp::Transform* trans2 = m_hoveredDefences.at(net.id).def1x3.GetComponent<comp::Transform>();
					if (trans2)
					{
						if (trans2->position != sm::Vector3(0, 0, 0))
						{
							trans2->position = sm::Vector3(0, 0, 0);
							entities.push_back(m_hoveredDefences.at(net.id).def1x3);
						}
					}
				}
			}
		});

	return entities;
}

std::vector<Entity> GridSystem::HideHoverDefence()
{
	std::vector<Entity> entities;

	for (auto& def : m_hoveredDefences)
	{
		comp::Transform* trans1 = def.second.def1x1.GetComponent<comp::Transform>();
		if (trans1)
		{
			if (trans1->position != sm::Vector3(0, 0, 0))
			{
				trans1->position = sm::Vector3(0, 0, 0);
				entities.push_back(def.second.def1x1);
			}
		}
		comp::Transform* trans2 = def.second.def1x3.GetComponent<comp::Transform>();
		if (trans2)
		{
			if (trans2->position != sm::Vector3(0, 0, 0))
			{
				trans2->position = sm::Vector3(0, 0, 0);
				entities.push_back(def.second.def1x3);
			}
		}
	}

	return entities;
}

bool GridSystem::RemoveDefence(Ray_t& mouseRay, uint32_t playerWhoPressedMouse, Blackboard* blackboard)
{
	sm::Vector3 localPlayer;
	comp::Player player;
	PathFinderManager* aiHandler = blackboard->GetPathFindManager();
	m_scene->ForEachComponent<comp::Player, comp::Transform, comp::Network>([&](comp::Player& p, comp::Transform& t, comp::Network& net)
		{
			if (net.id == playerWhoPressedMouse)
			{
				localPlayer = t.position;
				player = p;
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
			}

			if (aiHandler->PlayerAStar(localPlayer))
				player.reachable = true;
		}

		aiHandler->RemoveDefenseEntity(closestEntity);
		closestEntity.Destroy();

		return true;
	}
	else
	{
		return false;
	}
}

void GridSystem::RemoveDefence(const Entity& entity, Blackboard* blackboard)
{
	comp::TileSet* tileset = entity.GetComponent<comp::TileSet>();
	if (tileset)
	{
		//Go throgh all the tiles and fix them
		for (size_t i = 0; i < tileset->coordinates.size(); i++)
		{
			int zpos = tileset->coordinates[i].first;
			int xpos = tileset->coordinates[i].second;
			m_tiles[zpos][xpos].type = TileType::EMPTY;
		}
	}

	blackboard->GetPathFindManager()->RemoveDefenseEntity(entity);
}

std::vector<std::pair<UINT, UINT>> GridSystem::CheckDefenceLocation(Ray_t& mouseRay, const uint32_t& playerID)
{
	bool okayToPlace = true;
	uint8_t playerIndex = -1;
	comp::Player player;

	std::vector<std::pair<UINT, UINT>> coordinates;
	std::vector<comp::SphereCollider> ePos;
	m_scene->ForEachComponent<comp::Player, comp::SphereCollider, comp::Network>([&](comp::Player& p, comp::SphereCollider bs, comp::Network& net)
		{
			if (net.id == playerID)
			{
				playerIndex = static_cast<uint8_t>(ePos.size());
				player = p;
			}
			ePos.push_back(bs);
		});
	m_scene->ForEachComponent<comp::NPC, comp::SphereCollider>([&](comp::NPC& p, comp::SphereCollider& bs)
		{
			ePos.push_back(bs);
		});

	m_scene->ForEachComponent<comp::Villager, comp::SphereCollider>([&](comp::Villager& v, comp::SphereCollider& bs)
		{
			ePos.push_back(bs);
		});

	if (playerIndex != -1)
	{
		Plane_t plane;
		plane.normal = { 0.0f, 1.0f, 0.0f };
		sm::Vector3 pos;
		if (mouseRay.Intersects(plane, &pos))
		{
			//Tile touched
			int centerTileX = static_cast<int>(std::abs(pos.x) / m_tileSize.x);
			int centerTileZ = static_cast<int>(std::abs(pos.z) / m_tileSize.y);

			//Check distance between player and the tile what we touched
			float tileToPlayerDistance = (ePos[playerIndex].Center - m_tiles[centerTileZ][centerTileX].position).Length();
			if (tileToPlayerDistance >= player.buildDistance)
				okayToPlace = false;

			UINT numberOfDefences = 0;
			ShopItem shopitem = player.shopItem;
			if (shopitem == ShopItem::Defence1x1)
				numberOfDefences = 1;
			else if (shopitem == ShopItem::Defence1x3)
				numberOfDefences = 3;

			//Check if it was okay to place all the defences here
			for (UINT d = 0; d < numberOfDefences && okayToPlace; d++)
			{
				int xPos = centerTileX;
				int zPos = centerTileZ;
				if (player.rotateDefence)
					zPos += TileOffset(d);
				else
					xPos += TileOffset(d);

				Tile tile = m_tiles[zPos][xPos];

				//Check the tiles current type
				if (tile.type != TileType::EMPTY)
					okayToPlace = false;

				coordinates.push_back({ zPos, xPos });

				//Make sure we are within the grid
				if (InsideGrid(xPos, zPos))
				{
					//Each side of the tile
					float right = tile.position.x + m_tileHalfWidth;
					float left = tile.position.x - m_tileHalfWidth;
					float top = tile.position.z + m_tileHalfWidth;
					float bottom = tile.position.z - m_tileHalfWidth;

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
				else
					okayToPlace = false;
			}
			if (numberOfDefences == 0)
				okayToPlace = false;
		}
	}
	else
	{
		okayToPlace = false;
	}

	//Okay to place so we return all the coordinates of the relevant tiles
	if (okayToPlace)
	{
		coordinates.shrink_to_fit();
		return coordinates;
	}
	else
	{
		coordinates.clear();
		return coordinates;
	}
}

bool GridSystem::PlaceDefence(Ray_t& mouseRay, uint32_t playerWhoPressedMouse, PathFinderManager* aiHandler, QuadTree* dynamicQT, Blackboard* blackboard)
{
	std::vector<std::pair<UINT, UINT>> coordinates = CheckDefenceLocation(mouseRay, playerWhoPressedMouse);

	//Check that it was okay to place here
	if (!coordinates.empty())
	{
		comp::Player player;
		comp::SphereCollider playerCollider;

		m_scene->ForEachComponent<comp::Player, comp::Network, comp::SphereCollider>([&](comp::Player& p, comp::Network& net, comp::SphereCollider& sc)
			{
				if (net.id == playerWhoPressedMouse)
				{
					player = p;
					playerCollider = sc;
				}
			});

		//Go through all the tiles and set them to defence
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
					connectionRemovalNode1->RemoveConnection(connectionRemovalNode2);
					connectionRemovalNode2->RemoveConnection(connectionRemovalNode1);
				}
			}
		}

		//A-star-check
		if (!aiHandler->PlayerAStar(playerCollider.Center))
			player.reachable = false;


		/*
			Create the model for this tiles
		*/
		Entity tileEntity = m_scene->CreateEntity();
		tileEntity.AddComponent<comp::Tag<TagType::STATIC>>();
		tileEntity.AddComponent<comp::Tag<TagType::DEFENCE>>();
		tileEntity.AddComponent<comp::Network>();
		tileEntity.AddComponent<comp::Cost>()->cost = 100;
		tileEntity.AddComponent<comp::TileSet>()->coordinates = coordinates;

		comp::Transform* transform = tileEntity.AddComponent<comp::Transform>();
		comp::OrientedBoxCollider* collider = tileEntity.AddComponent<comp::OrientedBoxCollider>();
		comp::Health* health = tileEntity.AddComponent<comp::Health>();

		blackboard->GetPathFindManager()->AddDefenseEntity(tileEntity);

		sm::Vector3 centerpoint = CalcCenterPoint(coordinates);
		transform->position = { centerpoint.x, 5.f, centerpoint.z };
		collider->Center = transform->position;

		UINT numberOfDefences = 0;
		ShopItem shopitem = player.shopItem;
		if (shopitem == ShopItem::Defence1x1)
			numberOfDefences = 1;
		else if (shopitem == ShopItem::Defence1x3)
			numberOfDefences = 3;

		if (numberOfDefences != 0)
		{
			if (player.rotateDefence)
			{
				transform->rotation = sm::Quaternion::CreateFromYawPitchRoll(static_cast<float>(PI / 2.f), 0.f, 0.f);
				collider->Extents = { m_tileHalfWidth, m_tileHalfWidth, m_tileHalfWidth * numberOfDefences };
			}
			else
			{
				collider->Extents = { m_tileHalfWidth * numberOfDefences, m_tileHalfWidth, m_tileHalfWidth };
			}

			if (shopitem == ShopItem::Defence1x1)
			{
				health->maxHealth = 100.0f;
				health->currentHealth = health->maxHealth;
				tileEntity.AddComponent<comp::MeshName>()->name = NameType::MESH_DEFENCE1X1;
			}
			else if (shopitem == ShopItem::Defence1x3)
			{
				health->maxHealth = 300.0f;
				health->currentHealth = health->maxHealth;
				tileEntity.AddComponent<comp::MeshName>()->name = NameType::MESH_DEFENCE1X3;
			}
			dynamicQT->Insert(tileEntity);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
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
