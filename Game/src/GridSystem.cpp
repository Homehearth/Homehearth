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

	for (int i = 0; i < m_gridSize.x * m_gridSize.y * 4; i += 4)
	{
		//Calculate how much % of white it is per pixel and divide it by 10 to give it a level value.
		int h = (std::round((pixels[i] / 255.f)));

		pixelValues.emplace_back(h);
	}


	for (int col = 0; col < m_gridSize.y; col++)
	{
		for (int row = 0; row < m_gridSize.x; row++)
		{

			//Initialize all tiles
			TileType tileTypeTemp;

			if (pixelValues.at(row + m_gridSize.x * col) == 1)//If pixel this color
			{
				tileTypeTemp = TileType::DEFAULT;
			}
			else //if(false) 
			{
				tileTypeTemp = TileType::BUILDING;
			}


			Tile tileTemp;
			tileTemp.Initialize({ (float)(m_mapSize.x/ m_gridSize.x),(float)(m_mapSize.y / m_gridSize.y) }, {(float)row, (float)col}, {0.f, 0.f, 0.0f}, tileTypeTemp);

			if (tileTypeTemp == TileType::DEFAULT)
			{
				std::cout << "Default tile on: " << tileTemp.GetGridID().x << " " << tileTemp.GetGridID().y << " with " << pixelValues.at(row + m_gridSize.x * col) << std::endl;
			}
			else 
			{
				std::cout << "Building tile on: " << tileTemp.GetGridID().x << " " << tileTemp.GetGridID().y << " with " << pixelValues.at(row + m_gridSize.x * col) << std::endl;
			}


		}
	}
}