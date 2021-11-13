#pragma once
#include <stb_image.h>
class AIHandler;

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
	TileType type					= TileType::DEFAULT;
	Vector2I gridID				= { -1, -1 };
	float halfWidth					= 0;
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

public:
									GridSystem();
									~GridSystem();

	/*GridSize is decided by the texture size, how many tiles in x and why, MapSize is how big the world is*/
	void							Initialize(Vector2I mapSize, sm::Vector3 position, std::string fileName, HeadlessScene* scene);

	void							PlaceDefence(Ray_t& mouseRay, uint32_t playerWhoPressedMouse, AIHandler* aiHandler);
	uint32_t						GetTileCount() const;
	//Get functions
	Vector2I						GetGridSize() const;
	Tile*							GetTile(Vector2I& id);
	std::vector<Entity>*			GetTileEntities();
};