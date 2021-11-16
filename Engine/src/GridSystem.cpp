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

			Tile tileTemp;
			tileTemp.gridID		= { col, row };
			tileTemp.halfWidth	= m_tileHalfWidth;
			tileTemp.type		= tileTypeTemp;
			tileTemp.position	= tilePosition;

			if (rowTilesTemp.size() < m_gridSize.x)
				rowTilesTemp.push_back(tileTemp);
			
			if (rowTilesTemp.size() >= m_gridSize.x)
				m_tiles.push_back(rowTilesTemp);

#if RENDER_GRID
			if(tileTemp.type == TileType::DEFAULT || tileTemp.type == TileType::EMPTY)
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


void GridSystem::PlaceDefence(Ray_t& mouseRay, uint32_t playerWhoPressedMouse, PathFinderManager* aiHandler)
{
	float t = 0;

	const float MAX_RADIUS = 40.f;
	const float MIN_RADIUS = 10.f;

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
							m_tiles[row][col].type = TileType::DEFENCE;

							Entity tileEntity = m_scene->CreateEntity();
							comp::Transform* transform = tileEntity.AddComponent<comp::Transform>();
							transform->position = tile.position;
							transform->position.y = 0.5;

							transform->scale = { 4.2f, 0.5f, 4.2f };
							tileEntity.AddComponent<comp::Network>();
							comp::BoundingOrientedBox* collider = tileEntity.AddComponent<comp::BoundingOrientedBox>();
							collider->Center = tileEntity.GetComponent<comp::Transform>()->position;
							collider->Extents = { tileEntity.GetComponent<comp::Transform>()->scale.x, 10.f , tileEntity.GetComponent<comp::Transform>()->scale.z };
							tileEntity.AddComponent<comp::Tag<TagType::STATIC>>();
							tileEntity.AddComponent<comp::MeshName>()->name = "Defence.obj";

							aiHandler->GetNodeByID(Vector2I(row, col))->reachable = false;
							aiHandler->GetNodeByID(Vector2I(row, col))->connections.clear();
						}
					}
					//else if (tile.type == TileType::BUILDING || tile.type == TileType::UNPLACABLE || tile.type == TileType::DEFAULT)
					//{
					//	LOG_INFO("You cant place here!");
					//}
					//else if (tile.type == TileType::DEFENCE)
					//{
					//	LOG_INFO("Theres already a defence here!");
					//}
				}
			}
		}
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
