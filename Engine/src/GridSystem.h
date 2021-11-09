#pragma once
#include "EnginePCH.h"
#include <stb_image.h>
#include "Intersections.h"

class GridSystem
{
private:
	Vector2I m_mapSize;	//Width x length in, ingame units
	Vector2I m_gridSize;	//Tiles x tiles
	sm::Vector3 m_position;

	float m_tileHalfWidth;
	HeadlessScene* m_scene;

	std::vector<sm::Vector3> m_tilePositions;
	std::vector<Entity> m_tiles;

public:
	GridSystem();
	~GridSystem();

	/*GridSize is decided by the texture size, how many tiles in x and why, MapSize is how big the world is*/
	void Initialize(Vector2I mapSize, sm::Vector3 position, std::string fileName, HeadlessScene* scene);

	uint32_t PlaceDefenceRenderGrid(Ray_t& mouseRay);
	sm::Vector3 PlaceDefence(Ray_t& mouseRay);

	//Get functions
	std::vector<sm::Vector3>* GetTilePositions();
	Vector2I GetGridSize() const;
	Entity* GetTileByID(Vector2I& id);
	std::vector<Entity>* GetTiles();
};