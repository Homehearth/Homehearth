#pragma once
#include "GridSystem.h"

class AIHandler
{
private:
	std::vector<std::vector<std::unique_ptr<Node>>> m_nodes;
	//std::unordered_map<Vector2I, std::unique_ptr<Node>> m_nodes;
	Entity FindClosestPlayer(HeadlessScene& scene, sm::Vector3 position, comp::NPC* npc);
	std::vector<Node*> GetNeighbors(GridSystem* grid, Tile* baseNode);
	Node* FindClosestNode(sm::Vector3 position);
public:
	AIHandler();
	virtual ~AIHandler();
	void SetClosestNode(comp::NPC& npc, sm::Vector3 position);
	Node* GetNodeByID(Vector2I id) const;
	bool AddNode(Vector2I id);
	bool AddNode(std::unique_ptr<Node>& node);
	void CreateNodes(GridSystem* grid);
	void AStarSearch(HeadlessScene& scene, Entity npc);
	bool ReachedNode(const Entity npc);

};
