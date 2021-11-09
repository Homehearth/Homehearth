#include "EnginePCH.h"
#include "GridSystem.h"
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
	sm::Vector2 tileSize = { (float)m_mapSize.x / m_gridSize.x, (float)m_mapSize.y / m_gridSize.y };

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

			m_tileHalfWidth = (tileSize.x / 2.f);
			sm::Vector3 tilePosition = { tileSize.x * row + m_tileHalfWidth, 0.f , (tileSize.y * -col) - m_tileHalfWidth };
			m_tilePositions.push_back(tilePosition);

			Tile tileTemp;
			tileTemp.gridID = { (float)row, (float)col };
			tileTemp.halfWidth = m_tileHalfWidth;
			tileTemp.type = tileTypeTemp;
			tileTemp.position = tilePosition;

#if RENDER_GRID
			if (tileTypeTemp != TileType::DEFAULT)
			{
				Entity tileEntity = m_scene->CreateEntity();
				comp::Tile* tile = tileEntity.AddComponent<comp::Tile>();
				tile->gridID = { (float)row, (float)col };
				tile->halfWidth = m_tileHalfWidth;
				tile->type = tileTypeTemp;
				comp::Transform* transform = tileEntity.AddComponent<comp::Transform>();
				transform->position = tilePosition;
				transform->position.y = 0.5;

				transform->scale = { 4.2f, 0.5f, 4.2f };
				tileEntity.AddComponent<comp::Network>();
			}
#endif

			if (rowTilesTemp.size() < m_gridSize.x)
				rowTilesTemp.push_back(tileTemp);

			if (rowTilesTemp.size() >= m_gridSize.x)
				m_tiles.push_back(rowTilesTemp);

		}
		rowTilesTemp.clear();
	}
	stbi_image_free(pixelsData);
}


uint32_t GridSystem::PlaceDefenceRenderGrid(Ray_t& mouseRay)
{
	float t = 0;

	int returnID = -1;

	Plane_t plane;
	plane.normal = { 0.0f, 1.0f, 0.0f };

	sm::Vector3 pos;
	bool isHit = false;

	if (mouseRay.Intersects(plane, &pos))
	{
		for (int col = 0; col < m_gridSize.y && !isHit; col++)
		{
			for (int row = 0; row < m_gridSize.x && !isHit; row++)
			{
				Tile tile = m_tiles[row][col];

				float right = tile.position.x + tile.halfWidth;
				float left = tile.position.x - tile.halfWidth;
				float top = tile.position.z + tile.halfWidth;
				float bottom = tile.position.z - tile.halfWidth;

				if (pos.x > left && pos.x < right && pos.z < top && pos.z > bottom)
				{
					isHit = true;
					if (m_tiles[row][col].type == TileType::EMPTY)
					{
						LOG_INFO("Mouseray HIT plane detected a EMPTY Tile!");
						m_tiles[row][col].type = TileType::DEFENCE;
#if RENDER_GRID
						if (entity.GetComponent<comp::Network>())
						{
							returnID = entity.GetComponent<comp::Network>()->id;
							comp::BoundingOrientedBox* collider = entity.AddComponent<comp::BoundingOrientedBox>();
							collider->Center = entity.GetComponent<comp::Transform>()->position;
							collider->Extents = { entity.GetComponent<comp::Transform>()->scale.x, 10.f , entity.GetComponent<comp::Transform>()->scale.z };
							entity.AddComponent<comp::Tag<TagType::STATIC>>();
						}
#endif // RENDER_GRID

					}
					else if (m_tiles[row][col].type == TileType::BUILDING || m_tiles[row][col].type == TileType::UNPLACABLE || m_tiles[row][col].type == TileType::DEFAULT)
					{
						LOG_INFO("You cant place here!");
					}
					else if (m_tiles[row][col].type == TileType::DEFENCE)
					{
						LOG_INFO("Theres already a defence here!");
					}
				}
			}
		}
	}

	return returnID;
}

sm::Vector3 GridSystem::PlaceDefence(Ray_t& mouseRay, uint32_t playerWhoPressedMouse)
{
	float t = 0;

	const float MAX_RADIUS = 40.f;
	const float MIN_RADIUS = 10.f;

	sm::Vector3 returnPosition = { -1, -1 ,-1 };
	Plane_t plane;
	plane.normal = { 0.0f, 1.0f, 0.0f };
	sm::Vector3 pos;
	bool tileFound = false;
	bool canBuild = false;

	sm::Vector3 localPlayer;
	std::vector<sm::Vector3> pPos;

	// Save positions to calculate distances to the tile
	m_scene->ForEachComponent<comp::Player, comp::Transform, comp::Network>([&](comp::Player& p, comp::Transform& t, comp::Network& net)
		{
			if (net.id != playerWhoPressedMouse)
			{
				pPos.push_back(t.position);
			}
			else
			{
				localPlayer = t.position;
			}
		});

	if (mouseRay.Intersects(plane, &pos))
	{
		for (int col = 0; col < m_gridSize.y && !tileFound; col++)
		{
			for (int row = 0; row < m_gridSize.x && !tileFound; row++)
			{
				Tile tile = m_tiles[row][col];

				float right = tile.position.x + tile.halfWidth;
				float left = tile.position.x - tile.halfWidth;
				float top = tile.position.z + tile.halfWidth;
				float bottom = tile.position.z - tile.halfWidth;

				// Is mouse position we clicked at within the tiles bounds
				if (pos.x > left && pos.x < right && pos.z < top && pos.z > bottom)
				{
					tileFound = true;
					if (tile.type == TileType::EMPTY)
					{
						bool tileOccupied = false;
						// Other players, should be other entities
						for (int i = 0; i < pPos.size() && !tileOccupied; i++)
						{
							if (sm::Vector3::Distance(pPos[i], tile.position) < MIN_RADIUS)
							{
								tileOccupied = true;
							}
						}
						if (!tileOccupied && 
							sm::Vector3::Distance(tile.position, localPlayer) < MAX_RADIUS && 
							sm::Vector3::Distance(tile.position, localPlayer) > MIN_RADIUS)
						{
							tile.type = TileType::DEFENCE;

							returnPosition = tile.position;
							Entity defenceEntity = m_scene->CreateEntity();
							comp::BoundingOrientedBox* collider = defenceEntity.AddComponent<comp::BoundingOrientedBox>();
							collider->Center = returnPosition;

							collider->Extents = { 4.2f, 10.f , 4.2f };
							defenceEntity.AddComponent<comp::Tag<TagType::STATIC>>();
						}
					}
					else if (tile.type == TileType::BUILDING || tile.type == TileType::UNPLACABLE || tile.type == TileType::DEFAULT)
					{
						LOG_INFO("You cant place here!");
					}
					else if (tile.type == TileType::DEFENCE)
					{
						LOG_INFO("Theres already a defence here!");
					}
				}
			}
		}
	}

	return returnPosition;
}

Vector2I GridSystem::GetGridSize() const
{
	return m_gridSize;
}

std::vector<sm::Vector3>* GridSystem::GetTilePositions()
{
	return &m_tilePositions;
}

Entity* GridSystem::GetTileByID(Vector2I& id)
{
	if (id.x >= 0 && id.y >= 0 && id.x < m_gridSize.x && id.y < m_gridSize.y)
	{
		for (int i = 0; i < m_tileEntites.size(); i++)
		{
			comp::Tile* tile = m_tileEntites.at(i).GetComponent<comp::Tile>();
			if (tile->gridID == id)
			{
				return &m_tileEntites.at(i);
			}
		}
	}
	return nullptr;
}

Tile GridSystem::GetTile(Vector2I& id)
{
	return m_tiles.at(id.x).at(id.y);
}

std::vector<Entity>* GridSystem::GetTileEntities()
{
	return nullptr;
}
