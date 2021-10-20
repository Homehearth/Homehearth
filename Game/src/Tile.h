#pragma once
#include "EnginePCH.h"

enum class TileType
{
	DEFAULT,
	OCCUPIED,
	BUILDING
};

class Tile
{
private:
	sm::Vector2 m_size;
	sm::Vector2 m_gridID;
	sm::Vector3 m_poisition;

	float m_halfWidth;
	float m_halfHeight;

	TileType m_type;

public:
	Tile();
	~Tile();

	void Initialize(sm::Vector2 size, sm::Vector2 gridID, sm::Vector3 poisition, TileType type);

	sm::Vector2 GetGridID();
};