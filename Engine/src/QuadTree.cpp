#include "EnginePCH.h"
#include "QuadTree.h"

QuadTree::QuadTree(dx::BoundingBox boundary, int level)
{
	m_boundary = boundary;
	m_divided = false;
	m_level = level;
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
	comp::OrientedBoxCollider* colliderBox = e.GetComponent<comp::OrientedBoxCollider>();
	if (colliderBox)
	{
		if (!m_boundary.Intersects(*colliderBox) || m_level > MAX_LEVELS)
		{
			return false;
		}
	}
	else
	{
		comp::SphereCollider* colliderSphere = e.GetComponent<comp::SphereCollider>();
		if (colliderSphere)
		{
			if (!m_boundary.Intersects(*colliderSphere) || m_level > MAX_LEVELS)
			{
				return false;
			}
		}
	}

	if (m_level == MAX_LEVELS)
	{
		m_entities.insert(e);
		return true;
	}

	// Divide the QuadTree into 4 new subsections and pass the leaf nodes the current quads items
	if (!m_divided && m_level < MAX_LEVELS)
	{
		Split();
	}

	this->NorthWest->Insert(e);
	this->NorthEast->Insert(e);
	this->SouthWest->Insert(e);
	this->SouthEast->Insert(e);

	return true;
}

void QuadTree::Clear()
{
	if (m_entities.size() > 0)
	{
		m_entities.clear();
	}

	if (m_divided && m_level <= MAX_LEVELS)
	{
		this->NorthWest->Clear();
		this->NorthEast->Clear();
		this->SouthWest->Clear();
		this->SouthEast->Clear();
	}
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

	this->NorthWest = std::make_unique<QuadTree>(nw, m_level + 1);
	this->NorthEast = std::make_unique<QuadTree>(ne, m_level + 1);
	this->SouthWest = std::make_unique<QuadTree>(sw, m_level + 1);
	this->SouthEast = std::make_unique<QuadTree>(se, m_level + 1);

	m_divided = true;
}

void QuadTree::Query(std::set<Entity>& returnVec, const comp::SphereCollider& range)
{
	if (!m_boundary.Intersects(range))
	{
		return;
	}

	for (auto entity : m_entities)
	{
		returnVec.insert(entity);
	}

	if (m_divided)
	{
		this->NorthWest->Query(returnVec, range);
		this->NorthEast->Query(returnVec, range);
		this->SouthWest->Query(returnVec, range);
		this->SouthEast->Query(returnVec, range);
	}
}