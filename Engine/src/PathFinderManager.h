#pragma once
#include "GridSystem.h"


class PathFinderManager
{
private:
	sm::Vector2 m_nodeSize;
	std::vector<std::vector<std::shared_ptr<Node>>> m_nodes;
	std::vector<Node*> GetNeighbors(GridSystem* grid, Tile* baseNode);
	Node* GetDistantNode(sm::Vector3 position);
	bool IsInVector(std::vector<Node*> vector, Node* node);
	//Stores all defense buildings player has placed
	std::unordered_map<Entity, Entity> defenseEntities;
public:
	PathFinderManager();
	virtual ~PathFinderManager();

	//Getters
	Node* GetNodeByID(Vector2I id) const;
	std::vector<std::vector<std::shared_ptr<Node>>>& GetNodes();
	[[nodiscard]] std::unordered_map<Entity, Entity> GetDefenseEntities();

	void AddDefenseEntity(Entity entity);
	void RemoveDefenseEntity(Entity entity);
	Node* AddNode(Vector2I id);
	void CreateNodes(GridSystem* grid);
	void AStarSearch(Entity npc);
	bool PlayerAStar(sm::Vector3 playerPos);
	bool ReachedNode(const Entity npc);
	Node* FindClosestNode(sm::Vector3 position);
	float GetNodeSize()const;
};

