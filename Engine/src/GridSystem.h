#pragma once
#include <stb_image.h>

class PathFinderManager;
class HeadlessScene;
class QuadTree;

enum class TileType
{
	DEFAULT,
	EMPTY,
	BUILDING,
	UNPLACABLE,
	DEFENCE
};

struct Tile 
{
	TileType	type				= TileType::DEFAULT;
	Vector2I	gridID				= { -1, -1 };
	sm::Vector3 position			= { 0, 0, 0 };
};

class GridSystem
{
private:
	Vector2I						m_mapSize;	//Width x length in, ingame units
	Vector2I						m_gridSize;	//Tiles x tiles
	sm::Vector3						m_position;
	float							m_tileHalfWidth;
	HeadlessScene*					m_scene;
	std::vector<std::vector<Tile>>	m_tiles;

private:
	bool							InsideGrid(const int& xpos, const int& zpos) const;
	//Get a tile offset of: 0, +1, -1, +2, -2, +3, -3... from the center
	int								TileOffset(const int& index) const;
	
public:
									GridSystem();
									~GridSystem();
	sm::Vector2						m_tileSize;

	//GridSize is decided by the texture size, how many tiles in x and why, MapSize is how big the world is
	void							Initialize(Vector2I mapSize, sm::Vector3 position, std::string fileName, HeadlessScene* scene);
	
	//Delete a defence where the mouse is
	bool							RemoveDefence(Ray_t& mouseRay, uint32_t playerWhoPressedMouse, PathFinderManager* aiHandler);
	
	//Delete the defence from an entity
	void							RemoveDefence(const Entity& entity);

	//Place defence where the mouse is
	bool							PlaceDefence(Ray_t& mouseRay, uint32_t playerWhoPressedMouse, PathFinderManager* aiHandler, QuadTree* dynamicQT);

	//Get functions
	uint32_t						GetTileCount() const;
	Vector2I						GetGridSize() const;
	Tile*							GetTile(Vector2I& id);
	std::vector<Entity>*			GetTileEntities();
};