#pragma once
#include "GridSystem.h"


class PathFinderManager
{
private:
	std::vector<std::vector<std::shared_ptr<Node>>> m_nodes;
	std::vector<Node*> GetNeighbors(GridSystem* grid, Tile* baseNode);
	Node* GetDistantNode(sm::Vector3 position);
	bool IsInVector(std::vector<Node*> vector, Node* node);
public:
	PathFinderManager();
	virtual ~PathFinderManager();
	Node* GetNodeByID(Vector2I id) const;
	Node* AddNode(Vector2I id);
	void CreateNodes(GridSystem* grid);
	void AStarSearch(Entity npc);
	bool PlayerAStar(sm::Vector3 playerPos);
	bool ReachedNode(const Entity npc);
	Node* FindClosestNode(sm::Vector3 position);
	std::vector<std::vector<std::shared_ptr<Node>>>& GetNodes();

};

