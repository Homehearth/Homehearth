#pragma once
#include <stb_image.h>

class Blackboard;
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

	struct defences
	{
		Entity def1x1;
		Entity def1x3;
	};
	std::unordered_map<uint32_t, defences> m_hoveredDefences;

private:
	bool							InsideGrid(const int& xpos, const int& zpos) const;
	//Get a tile offset of: 0, +1, -1, +2, -2, +3, -3... from the center
	int								TileOffset(const int& index) const;
	//Send in what tiles we are checking and calculate the centerpoint of them
	sm::Vector3						CalcCenterPoint(const std::vector<std::pair<UINT, UINT>>& coordinates);
	
public:
									GridSystem();
									~GridSystem();
	sm::Vector2						m_tileSize;

	//GridSize is decided by the texture size, how many tiles in x and why, MapSize is how big the world is
	void							Initialize(Vector2I mapSize, sm::Vector3 position, std::string fileName, HeadlessScene* scene);
	
	std::vector<Entity>				UpdateHoverDefence();
	std::vector<Entity>				HideHoverDefence();

	//Delete a defence where the mouse is
	bool							RemoveDefence(Ray_t& mouseRay, uint32_t playerWhoPressedMouse, Blackboard* blackboard);
	
	//Delete the defence from an entity
	void							RemoveDefence(const Entity& entity, Blackboard* blackboard);

	//Check if it's okay to place a defence on this location
	//Return a vector of all the locations that was okay
	std::vector<std::pair<UINT, UINT>>	CheckDefenceLocation(Ray_t& mouseRay, const uint32_t& playerID);


	//Place defence where the mouse is
	bool							PlaceDefence(Ray_t& mouseRay, uint32_t playerWhoPressedMouse, PathFinderManager* aiHandler, QuadTree* dynamicQT, Blackboard* blackboard);

	//Get functions
	uint32_t						GetTileCount() const;
	Vector2I						GetGridSize() const;
	Tile*							GetTile(Vector2I& id);
	std::vector<Entity>*			GetTileEntities();
};