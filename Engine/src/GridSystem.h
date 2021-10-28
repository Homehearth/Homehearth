#pragma once
#include "EnginePCH.h"
#include "Tile.h"

class GridSystem
{
private:
	sm::Vector2 m_mapSize;	//Width x length in, ingame units
	sm::Vector2 m_gridSize;	//Tiles x tiles
	sm::Vector3 m_position;

	std::vector<Tile> m_tiles;
	std::vector<sm::Vector3> m_tilePosiitons;

	bool m_isRenderingGrid;

public:
	GridSystem();
	~GridSystem();

	/*GridSize is decided by the texture size, how many tiles in x and why, MapSize is how big the world is*/
	void Initialize(sm::Vector2 mapSize, sm::Vector3 position, std::string fileName);
	std::vector<sm::Vector3>* GetTilePositions();
	std::vector<Tile>* GetTiles();
	bool GetIsRendering();
};