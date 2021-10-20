#pragma once
#include "EnginePCH.h"
#include "Tile.h"

class GridSystem
{
private:
	sm::Vector2 m_mapSize;	//Width x length in, ingame units
	sm::Vector3 m_gridSize;	//Tiles x tiles
	sm::Vector2 m_position;
	
	std::vector<Tile> tiles;

public:
	GridSystem();
	~GridSystem();

	void Initialize();


};