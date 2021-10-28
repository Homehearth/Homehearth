#include "EnginePCH.h"
#include "GridSystem.h"

GridSystem::GridSystem()
{

}

GridSystem::~GridSystem()
{
}

void GridSystem::Initialize(sm::Vector2 mapSize, sm::Vector3 position, std::string fileName)
{
	m_position = position;
	m_mapSize = mapSize;

	//std::shared_ptr<RTexture> texture = ResourceManager::Get().GetResource<RTexture>(fileName);

	std::shared_ptr<RTexture> texture = std::make_shared<RTexture>();
	unsigned char* pixelsData = texture->GetImageData(fileName);

	bool added = ResourceManager::Get().AddResource(fileName, texture);

	std::vector<int> pixelValues;
	m_gridSize = texture->GetSize();

	for (int i = 0; i < m_gridSize.x * m_gridSize.y * 4; i++)
	{
		pixelValues.push_back((int)pixelsData[i]);
		//std::cout << (int)pixelsData[i] << "  " << (int)pixelsData[i + 1] << "  " << (int)pixelsData[i + 2] << "  " << (int)pixelsData[i+3] << std::endl;
	}

	//Initialize all tiles
	for (int col = 0; col < m_gridSize.y; col++)
	{
		for (int row = 0; row < m_gridSize.x; row++)
		{
			TileType tileTypeTemp = TileType::DEFAULT;

			sm::Vector4 rgba;

			rgba.x = pixelValues.at(0 + (row + (col * m_gridSize.y)) * 4);
			rgba.y = pixelValues.at(1 + (row + (col * m_gridSize.y)) * 4);
			rgba.z = pixelValues.at(2 + (row + (col * m_gridSize.y)) * 4);
			rgba.w = pixelValues.at(3 + (row + (col * m_gridSize.y)) * 4);

			//std::cout << "RGBA: " << rgba.x << " " << rgba.y << " " << rgba.z << " " << rgba.w << std::endl;

			if (rgba == sm::Vector4{ 0, 255, 0, 255 }) // If Green
			{
				tileTypeTemp = TileType::EMPTY;
				std::cout << "  Empty    tile on: " << (float)row << " " << (float)col << "   with " << ", RGBA: " << rgba.x << " " << rgba.y << " " << rgba.z << " " << rgba.w << std::endl;
			}
			if (rgba == sm::Vector4{ 255, 0, 0, 255 }) // if Red
			{
				tileTypeTemp = TileType::BUILDING;
				std::cout << " Building  tile on: " << (float)row << " " << (float)col << "   with " << ", RGBA: " << rgba.x << " " << rgba.y << " " << rgba.z << " " << rgba.w << std::endl;
			}
			if (rgba == sm::Vector4{ 0, 0, 255, 255 }) // if Blue
			{
				tileTypeTemp = TileType::DEFENCE;
				std::cout << " Defence  tile on: " << (float)row << " " << (float)col << "   with " << ", RGBA: " << rgba.x << " " << rgba.y << " " << rgba.z << " " << rgba.w << std::endl;
			}

			Tile tileTemp;
			tileTemp.Initialize({ (float)(m_mapSize.x / m_gridSize.x),(float)(m_mapSize.y / m_gridSize.y) }, { (float)row, (float)col }, { 0.0f, 0.0f, 0.0f }, tileTypeTemp);
			m_tiles.push_back(tileTemp);

			float tileHalfWidth = tileTemp.GetHalfWidth();
			sm::Vector3 tilePosition = { tileHalfWidth + tileHalfWidth * row + m_position.x, 0.f + m_position.y, tileHalfWidth + tileHalfWidth * col + m_position.z };
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

bool GridSystem::GetIsRendering()
{
	return &m_isRenderingGrid;
}