#include "Tile.h"

Tile::Tile()
{
	m_halfWidth = m_size.x * 0.5f;
	m_halfHeight = m_size.y * 0.5f;
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
	m_halfWidth = m_size.x * 0.5f;
	m_halfHeight = m_size.y * 0.5f;
}
