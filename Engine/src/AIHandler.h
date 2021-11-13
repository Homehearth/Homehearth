#pragma once
#include "GridSystem.h"


class AIHandler
{
private:
	std::vector<std::vector<std::shared_ptr<Node>>> m_nodes;
	//std::unordered_map<Vector2I, std::unique_ptr<Node>> m_nodes;
	std::vector<Node*> GetNeighbors(GridSystem* grid, Tile* baseNode);
	
	float CalculateFGH(Node* currentNode, Node* neighbor, Node* goalNode);
	bool IsInVector(std::vector<Node*> vector, Node* node);
public:
	AIHandler();
	virtual ~AIHandler();
	Node* GetNodeByID(Vector2I id) const;
	Node* AddNode(Vector2I id);
	bool AddNode(std::unique_ptr<Node>& node);
	void CreateNodes(GridSystem* grid);
	void AStarSearch(Entity npc);
	bool ReachedNode(const Entity npc);
	Node* FindClosestNode(sm::Vector3 position);
	std::vector<std::vector<std::shared_ptr<Node>>>& GetNodes();

};

