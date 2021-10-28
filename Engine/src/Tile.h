#pragma once
#include "EnginePCH.h"

enum class TileType
{
	DEFAULT,
	EMPTY,
	BUILDING,
	DEFENCE
};

class Tile
{
private:
	sm::Vector2 m_size;
	sm::Vector2 m_gridID;
	sm::Vector3 m_poisition;

	float m_halfWidth;

	TileType m_type;

public:
	Tile();
	~Tile();

	void Initialize(sm::Vector2 size, sm::Vector2 gridID, sm::Vector3 poisition, TileType type);

	//Set functions
	void SetPosition(sm::Vector3 position);

	//Get functions
	sm::Vector2 GetGridID();
	float GetHalfWidth();
	TileType GetType();


};