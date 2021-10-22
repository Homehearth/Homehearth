#include "GridSystem.h"

GridSystem::GridSystem()
{
	//TODO: remove hårdkådning
	m_gridSize	= { 70.0f,	70.0f };
	m_mapSize	= { 600.0f, 600.0f };
	m_position	= { 0.0f,	0.0f };
}

GridSystem::~GridSystem()
{
}

void GridSystem::Initialize()
{
	std::shared_ptr<RTexture> texture = ResourceManager::Get().GetResource<RTexture>("GridMap.png");
	unsigned char* pixels = texture->GetImageData();
	std::vector<int> pixelValues;
	m_gridSize = texture->GetSize();

	for (int i = 0; i < m_gridSize.x * m_gridSize.y * 4; i+=4)
	{
		//std::cout << "Nr: " << i/4 << " " <<(int)pixels[i] << std::endl;
		pixelValues.push_back(pixels[i]);
	}

	//Initialize all tiles
	for (int col = 0; col < m_gridSize.y; col++)
	{
		for (int row = 0; row < m_gridSize.x; row++)
		{
			TileType tileTypeTemp = TileType::DEFAULT;
			float temp = (pixelValues.at((float)row + ((float)col * m_gridSize.y)));

			if (temp == 0) // If r channel is 0 
			{
				tileTypeTemp = TileType::EMPTY;
				std::cout << "  Empty    tile on: " << (float)row << " " << (float)col << "   with " << temp << std::endl;
			}
			else if (temp == 255) // if r channel is 255
			{
				tileTypeTemp = TileType::BUILDING;
				std::cout << " Building  tile on: " << (float)row << " " << (float)col << "   with " << temp << std::endl;
			}

			Tile tileTemp;
			tileTemp.Initialize({ (float)(m_mapSize.x / m_gridSize.x),(float)(m_mapSize.y / m_gridSize.y) }, { (float)row, (float)col }, {0.0f, 0.0f, 0.0f}, tileTypeTemp);
			m_tiles.push_back(tileTemp);

			float tileHalfWidth = tileTemp.GetHalfWidth();
			sm::Vector3 tilePosition = { tileHalfWidth + tileHalfWidth * row, 0.f, tileHalfWidth + tileHalfWidth * col };
			m_tilePosiitons.push_back(tilePosition);
			tileTemp.SetPosition(tilePosition);
		}
	}
}

std::vector<sm::Vector3>* GridSystem::GetTilePositions()
{
	return &m_tilePosiitons;
}

std::vector<Tile>* GridSystem::GetTiles()
{
	return &m_tiles;
}
