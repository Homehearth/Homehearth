#include "EnginePCH.h"
#include "Tile.h"

Tile::Tile()
{
	m_halfWidth = m_size.x * 0.5f;
	m_type = TileType::DEFAULT;
}

Tile::~Tile()
{
}

void Tile::Initialize(sm::Vector2 size, sm::Vector2 gridID, sm::Vector3 poisition, TileType type)
{
	m_size = size;
	m_gridID = gridID;
	m_poisition = poisition;
	m_type = type;
	m_halfWidth = m_size.x /2;
}

void Tile::SetPosition(sm::Vector3 position)
{
	m_poisition = position;
}

sm::Vector2 Tile::GetGridID()
{
	return m_gridID;
}

float Tile::GetHalfWidth()
{
	return m_halfWidth;
}

TileType Tile::GetType()
{
	return m_type;
}