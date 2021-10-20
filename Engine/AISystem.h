#pragma once
class AISystem
{
private:
	struct Node
	{
		float f, g, h;
		sm::Vector3 pos;
	};
	std::unordered_map<entt::entity, int> m_AIEntities;
	std::vector<Node> nodes;
public:
	AISystem();
	void GeneratePotentialFields();
	void GenerateNodes();
	void AStarSearch();
};

