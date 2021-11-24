#pragma once

class QuadTree
{
private:
	static const int				MAX_OBJECTS = 5;
	std::vector<Entity>				m_entities;
	dx::BoundingBox					m_boundary;
	bool							m_divided;

	std::unique_ptr<QuadTree>		NorthWest;
	std::unique_ptr<QuadTree>		NorthEast;
	std::unique_ptr<QuadTree>		SouthWest;
	std::unique_ptr<QuadTree>		SouthEast;

	void Split();

public:
	QuadTree(dx::BoundingBox boundary);
	~QuadTree();

	bool Insert(const Entity& e);
	void Query(std::vector<Entity>& returnVec, const dx::BoundingSphere& range);
	void GetSize(size_t& size);
};