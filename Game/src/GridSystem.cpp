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

void GridSystem::Initialize(std::string fileName)
{
	std::shared_ptr<RTexture> texture = ResourceManager::Get().GetResource<RTexture>(fileName);
	unsigned char* pixelsData = texture->GetImageData();
	std::vector<int> pixelValues;
	m_gridSize = texture->GetSize();

	for (int i = 0; i < m_gridSize.x * m_gridSize.y * 4; i++)
	{
		pixelValues.push_back(pixelsData[i]);
		//std::cout << (int)pixelsData[i] << "  " << (int)pixelsData[i + 1] << "  " << (int)pixelsData[i + 2] << "  " << (int)pixelsData[i+3] << std::endl;
	}

	//Initialize all tiles
	for (int col = 0; col < m_gridSize.y; col++)
	{
		for (int row = 0; row < m_gridSize.x; row++)
		{
			TileType tileTypeTemp = TileType::DEFAULT;

			sm::Vector4 rgba; 
			/*float rgbaIndex[4]; 

			rgbaIndex[0] = (0 + (row + (col * m_gridSize.y))*4);
			rgbaIndex[1] = (1 + (row + (col * m_gridSize.y))*4);
			rgbaIndex[2] = (2 + (row + (col * m_gridSize.y))*4);
			rgbaIndex[3] = (3 + (row + (col * m_gridSize.y))*4);
			*/
			rgba.x = pixelValues.at(0 + (row + (col * m_gridSize.y)) * 4);
			rgba.y = pixelValues.at(1 + (row + (col * m_gridSize.y)) * 4);
			rgba.z = pixelValues.at(2 + (row + (col * m_gridSize.y)) * 4);
			rgba.w = pixelValues.at(3 + (row + (col * m_gridSize.y)) * 4);

			//std::cout << "RGBA: " << rgba.x << " " << rgba.y << " " << rgba.z << " " << rgba.w << std::endl;

			if (rgba == sm::Vector4{0, 255, 0, 255}) // If Green
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

bool GridSystem::GetIsRendering()
{
	return &m_isRenderingGrid;
}
