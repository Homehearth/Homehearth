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


	for (int col = 0; col < m_gridSize.y; col++)
	{
		for (int row = 0; row < m_gridSize.x; row++)
		{
			//Initialize all tiles
			TileType tileTypeTemp;

			if (true)//If pixel this color
			{
				tileTypeTemp = TileType::DEFAULT;
			}
			else if(false) 
			{
				tileTypeTemp = TileType::BUILDING;
			}


			Tile tileTemp;
			tileTemp.Initialize({ (float)(m_mapSize.x/ m_gridSize.x),(float)(m_mapSize.y / m_gridSize.y) }, {row, col}, {0.f, 0.f, 0.0f}, tileTypeTemp);

		}
	}
}