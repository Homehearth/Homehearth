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
			tileTemp.gridID		= { (float)row, (float)col };
			tileTemp.halfWidth	= m_tileHalfWidth;
			tileTemp.type		= tileTypeTemp;
			tileTemp.position	= tilePosition;

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

	m_scene->ForEachComponent<comp::Tile>([&](Entity entity, comp::Tile& tile)
		{
			if (mouseRay.Intersects(plane, &plane.point))
			{
				if (tile.type == TileType::EMPTY)
				{
					LOG_INFO("Mouseray HIT plane detected a EMPTY Tile!");
					tile.type = TileType::DEFENCE;

					if (entity.GetComponent<comp::Network>())
					{
						returnID = entity.GetComponent<comp::Network>()->id;
						comp::BoundingOrientedBox* collider = entity.AddComponent<comp::BoundingOrientedBox>();
						collider->Center = entity.GetComponent<comp::Transform>()->position;
						collider->Extents = { entity.GetComponent<comp::Transform>()->scale.x, 10.f , entity.GetComponent<comp::Transform>()->scale.z };
						entity.AddComponent<comp::Tag<TagType::STATIC>>();
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
		});

	return returnID;
}

sm::Vector3 GridSystem::PlaceDefence(Ray_t& mouseRay)
{
	float t = 0;

	sm::Vector3 returnPosition = { -1, -1 ,-1 };
	Plane_t plane;
	plane.normal = { 0.0f, 1.0f, 0.0f };
	sm::Vector3 pos;

	m_scene->ForEachComponent<comp::Tile>([&](Entity entity, comp::Tile& tile)
		{
			if (mouseRay.Intersects(plane, &pos))
			{
				comp::Transform* t = entity.GetComponent<comp::Transform>();

				float right = t->position.x + tile.halfWidth;
				float left = t->position.x - tile.halfWidth;
				float top = t->position.z + tile.halfWidth;
				float bottom = t->position.z - tile.halfWidth;

				if (pos.x > left && pos.x < right && pos.z < top && pos.z > bottom)
				{
					if (tile.type == TileType::EMPTY)
					{
						LOG_INFO("Mouseray HIT plane detected a EMPTY Tile!");
						tile.type = TileType::DEFENCE;

						returnPosition = entity.GetComponent<comp::Transform>()->position;
						comp::BoundingOrientedBox* collider = entity.AddComponent<comp::BoundingOrientedBox>();
						collider->Center = returnPosition;
						collider->Extents = { entity.GetComponent<comp::Transform>()->scale.x, 10.f , entity.GetComponent<comp::Transform>()->scale.z };
						entity.AddComponent<comp::Tag<TagType::STATIC>>();
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
		});

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

Tile GridSystem::GetTile(sm::Vector2 id)
{
	return m_tiles.at(id.x).at(id.y);
}

std::vector<Entity>* GridSystem::GetTileEntities()
{
	return &m_tileEntites;
}
