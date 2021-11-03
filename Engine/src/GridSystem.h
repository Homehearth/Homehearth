#pragma once
#include "EnginePCH.h"
#include <stb_image.h>

class GridSystem
{
private:
	sm::Vector2 m_mapSize;	//Width x length in, ingame units
	sm::Vector2 m_gridSize;	//Tiles x tiles
	sm::Vector3 m_position;

	std::vector<sm::Vector3> m_tilePosiitons;
	std::vector<Entity> m_tiles;

public:
	GridSystem();
	~GridSystem();

	/*GridSize is decided by the texture size, how many tiles in x and why, MapSize is how big the world is*/
	void Initialize(sm::Vector2 mapSize, sm::Vector3 position, std::string fileName, HeadlessScene* scene);
	void Initialize2(sm::Vector2 mapSize, sm::Vector3 position, std::string fileName, Scene* scene); //TODO: remove when networking branch is in master
	std::vector<sm::Vector3>* GetTilePositions();
	sm::Vector2 GetGridSize() const;
	Entity* GetTileByID(sm::Vector2 id);
	std::vector<Entity>* GetTiles();
};