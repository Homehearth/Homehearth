#pragma once

class QuadTree
{
private:
	static const int				MAX_LEVELS = 5;
	static const int				MAX_OBJECTS = 3;
	std::set<Entity>				m_entities;
	dx::BoundingBox					m_boundary;
	bool							m_divided;
	int								m_level;

	std::unique_ptr<QuadTree>		NorthWest;
	std::unique_ptr<QuadTree>		NorthEast;
	std::unique_ptr<QuadTree>		SouthWest;
	std::unique_ptr<QuadTree>		SouthEast;

	void Split();

public:
	QuadTree(dx::BoundingBox boundary, int level = 0);
	~QuadTree();

	bool Insert(const Entity& e);
	void Query(std::set<Entity>& returnVec, const dx::BoundingSphere& range);
	void GetSize(size_t& size);
};