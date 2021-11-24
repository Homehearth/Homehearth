#include "EnginePCH.h"
#include "QuadTree.h"

QuadTree::QuadTree(dx::BoundingBox boundary)
{
	m_boundary = boundary;
	m_divided = false;
}

QuadTree::~QuadTree()
{

}

void QuadTree::GetSize(size_t& size)
{
	if (m_entities.size() == 0)
	{
		return;
	}

	size += m_entities.size();

	if (m_divided)
	{
		NorthWest->GetSize(size);
		NorthEast->GetSize(size);
		SouthWest->GetSize(size);
		SouthEast->GetSize(size);
	}
}

bool QuadTree::Insert(const Entity& e)
{
	dx::BoundingOrientedBox collider = *e.GetComponent<comp::BoundingOrientedBox>();

	if (m_boundary.Contains(collider) == dx::ContainmentType::DISJOINT)
	{
		return false;
	}

	if (m_entities.size() < MAX_OBJECTS)
	{
		m_entities.push_back(e);
		return true;
	}

	// Divive the QuadTree into 4 new subsections and pass the leaf nodes the current quads items
	if (!m_divided)
	{
		Split();

		for (size_t i = 0; i < m_entities.size(); i++)
		{
			this->Insert(std::move(m_entities[i]));
		}
		m_entities.clear();
	}

	if (this->NorthWest->Insert(e) || this->NorthEast->Insert(e) ||
		this->SouthWest->Insert(e) || this->SouthEast->Insert(e))
	{
		return true;
	}

	return false;
}

void QuadTree::Split()
{
	float width = m_boundary.Extents.x * 0.5f;
	float height = m_boundary.Extents.y;
	float deep = m_boundary.Extents.z * 0.5f;
	float x = m_boundary.Center.x;
	float y = m_boundary.Center.y;
	float z = m_boundary.Center.z;

	dx::BoundingBox nw(dx::XMFLOAT3(x - width, y, z + deep), dx::XMFLOAT3(width, height, deep));
	dx::BoundingBox ne(dx::XMFLOAT3(x + width, y, z + deep), dx::XMFLOAT3(width, height, deep));
	dx::BoundingBox sw(dx::XMFLOAT3(x - width, y, z - deep), dx::XMFLOAT3(width, height, deep));
	dx::BoundingBox se(dx::XMFLOAT3(x + width, y, z - deep), dx::XMFLOAT3(width, height, deep));

	this->NorthWest = std::make_unique<QuadTree>(nw);
	this->NorthEast = std::make_unique<QuadTree>(ne);
	this->SouthWest = std::make_unique<QuadTree>(sw);
	this->SouthEast = std::make_unique<QuadTree>(se);

	m_divided = true;
}

void QuadTree::Query(std::vector<Entity>& returnVec, const dx::BoundingSphere& range)
{
	if (!m_boundary.Contains(range))
	{
		return;
	}

	for (size_t i = 0; i < m_entities.size(); i++)
	{
		returnVec.push_back(m_entities[i]);
	}

	if (m_divided)
	{
		this->NorthWest->Query(returnVec, range);
		this->NorthEast->Query(returnVec, range);
		this->SouthWest->Query(returnVec, range);
		this->SouthEast->Query(returnVec, range);
	}
}