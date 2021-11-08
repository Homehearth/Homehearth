#pragma once
#include "EnginePCH.h"
#include "GridSystem.h"

class AIHandler
{
public:
	struct Node
	{
		float f = FLT_MAX, g = FLT_MAX, h = FLT_MAX;
		sm::Vector3 position;
		Vector2I id;
		std::vector<Node*> connections;
		Node* parent;
		bool reachable = true;
		Node(Vector2I id) : id(id) {};

		void ResetFGH()
		{
			f = FLT_MAX, g = FLT_MAX, h = FLT_MAX;
		}
		bool ConnectionAlreadyExists(Node* other)
		{
			for (Node* node : connections)
			{
				if (node == other)
				{
					return true;
				}
			}
			return false;
		}
	};
private:

	std::unordered_map<Vector2I, std::unique_ptr<Node>> m_nodes;
public:
	Node* GetNodeByID(Vector2I id) const;
	bool AddNode(Vector2I id);
	bool AddNode(std::unique_ptr<Node>& node);
	void CreateNodes(GridSystem* grid, HeadlessScene* scene);
	AIHandler();
	virtual ~AIHandler();
	void AStarSearch(HeadlessScene& scene, Entity npc);

};

