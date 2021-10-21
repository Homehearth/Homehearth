#pragma once
#include "EnginePCH.h"
#include "Tile.h"

class GridSystem
{
private:
	sm::Vector2 m_mapSize;	//Width x length in, ingame units
	sm::Vector2 m_gridSize;	//Tiles x tiles
	sm::Vector2 m_position;
	
	std::vector<Tile> m_tiles;
	std::vector<sm::Vector3> m_tilePosiitons;

public:
	GridSystem();
	~GridSystem();

	void Initialize();
	std::vector<sm::Vector3>* GetTilePositions();

};