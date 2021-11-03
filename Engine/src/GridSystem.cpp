#include "EnginePCH.h"
#include "GridSystem.h"
#define STB_IMAGE_IMPLEMENTATION

GridSystem::GridSystem()
{

}

GridSystem::~GridSystem()
{
}

void GridSystem::Initialize(sm::Vector2 mapSize, sm::Vector3 position, std::string fileName, HeadlessScene* scene)
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
	m_gridSize = { (float)width, (float)height };
	sm::Vector2 tileSize = { (float)(m_mapSize.x / m_gridSize.x),(float)(m_mapSize.y / m_gridSize.y) };

	for (int i = 0; i < m_gridSize.x * m_gridSize.y * 4; i++)
	{
		pixelValues.push_back((int)pixelsData[i]);
	}

	//Initialize all tiles
	for (int col = 0; col < m_gridSize.y; col++)
	{
		for (int row = 0; row < m_gridSize.x; row++)
		{
			TileType tileTypeTemp = TileType::DEFAULT;

			sm::Vector4 rgba;

			rgba.x = (float)pixelValues.at(0 + ((row + (col * (size_t)m_gridSize.y)) * 4));
			rgba.y = (float)pixelValues.at(1 + ((row + (col * (size_t)m_gridSize.y)) * 4));
			rgba.z = (float)pixelValues.at(2 + ((row + (col * (size_t)m_gridSize.y)) * 4));
			rgba.w = (float)pixelValues.at(3 + ((row + (col * (size_t)m_gridSize.y)) * 4));

			//std::cout << "RGBA: " << rgba.x << " " << rgba.y << " " << rgba.z << " " << rgba.w << std::endl;

			if (rgba == sm::Vector4{ 100, 100, 100, 255 })
				tileTypeTemp = TileType::EMPTY;

			if (rgba == sm::Vector4{ 255, 51, 0, 255 })
				tileTypeTemp = TileType::BUILDING;

			if (rgba == sm::Vector4{ 255, 234, 0, 255 } || rgba == sm::Vector4{ 0, 21, 255, 255 })
				tileTypeTemp = TileType::UNPLACABLE;

			if (rgba == sm::Vector4{ 0, 0, 0, 255 })
				tileTypeTemp = TileType::DEFAULT;

			m_tileHalfWidth = (float)(tileSize.x/2);
			sm::Vector3 tilePosition = {m_position.x + (m_tileHalfWidth * row) + m_tileHalfWidth/2, m_position.y + 0.f , -(m_tileHalfWidth * col) - (m_tileHalfWidth/2) + m_position.z};
			m_tilePosiitons.push_back(tilePosition);

			Entity tileEntity = m_scene->CreateEntity();
			comp::Tile* tile = tileEntity.AddComponent<comp::Tile>();
			tile->gridID = { (float)row, (float)col };
			tile->halfWidth = m_tileHalfWidth;
			tile->type = tileTypeTemp;
			comp::Transform* transform = tileEntity.AddComponent<comp::Transform>();
			transform->position = tilePosition;
			transform->position.y = 0.5;

			transform->scale = { 4.2f, 0.5f, 4.2f };

			if (tileTypeTemp != TileType::DEFAULT || RENDER_GRID)
				tileEntity.AddComponent<comp::Network>();

			comp::PlaneCollider* collider = tileEntity.AddComponent<comp::PlaneCollider>();
			collider->center = tilePosition;
			collider->size = tileSize;
			collider->normal = { 0,1,0 };

			m_tiles.push_back(tileEntity);
		}
	}
	stbi_image_free(pixelsData);
}


uint32_t GridSystem::PlaceDefenceRenderGrid(Ray_t mouseRay)
{
	float t = 0;

	int returnID = -1;

	m_scene->ForEachComponent<comp::Tile, comp::PlaneCollider>([&](Entity entity, comp::Tile& tile, comp::PlaneCollider& planeCollider)
		{
			if (Intersect::RayIntersectPlane(mouseRay,  planeCollider, t))
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

sm::Vector3 GridSystem::PlaceDefence(Ray_t mouseRay)
{
	float t = 0;

	sm::Vector3 returnPosition = {-1, -1 ,-1};

	m_scene->ForEachComponent<comp::Tile, comp::PlaneCollider>([&](Entity entity, comp::Tile& tile, comp::PlaneCollider& planeCollider)
		{
			if (Intersect::RayIntersectPlane(mouseRay, planeCollider, t))
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
		});

	return returnPosition;
}

std::vector<sm::Vector3>* GridSystem::GetTilePositions()
{
	return &m_tilePosiitons;
}

std::vector<Entity>* GridSystem::GetTiles()
{
	return &m_tiles;
}
