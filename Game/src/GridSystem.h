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

	bool m_isRenderingGrid;

public:
	GridSystem();
	~GridSystem();

	void Initialize(std::string fileName);
	std::vector<sm::Vector3>* GetTilePositions();
	std::vector<Tile>* GetTiles();
	bool GetIsRendering();
};