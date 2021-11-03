#pragma once
#include "EnginePCH.h"
#include <stb_image.h>
#include "Intersections.h"

class GridSystem
{
private:
	sm::Vector2 m_mapSize;	//Width x length in, ingame units
	sm::Vector2 m_gridSize;	//Tiles x tiles
	sm::Vector3 m_position;

	float m_tileHalfWidth;
	HeadlessScene* m_scene;

	std::vector<sm::Vector3> m_tilePosiitons;
	std::vector<Entity> m_tiles;

public:
	GridSystem();
	~GridSystem();

	/*GridSize is decided by the texture size, how many tiles in x and why, MapSize is how big the world is*/
	void Initialize(sm::Vector2 mapSize, sm::Vector3 position, std::string fileName, HeadlessScene* scene);

	uint32_t PlaceDefenceRenderGrid(Ray_t mouseRay);
	sm::Vector3 PlaceDefence(Ray_t mouseRay);

	//Get functions
	std::vector<sm::Vector3>* GetTilePositions();
	sm::Vector2 GetGridSize() const;
	Entity* GetTileByID(sm::Vector2 id);
	std::vector<Entity>* GetTiles();
};