#include "EnginePCH.h"
#include "NonDeterministicNodeBT.h"
#include <random>
#include <algorithm>

void BT::NonDeterministicNodeBT::ShuffleChildrenNodes()
{
	auto rng = std::default_random_engine{};
	std::shuffle(std::begin(m_childrenNodes), std::end(m_childrenNodes), rng);
}

BT::NonDeterministicNodeBT::NonDeterministicNodeBT(std::string name)
	:ParentNode(name)
{
	ShuffleChildrenNodes();
}

BT::NodeStatus BT::NonDeterministicNodeBT::Tick()
{
	for (const auto childNode : m_childrenNodes)
	{
		childStatus = childNode->Tick();

		if(childNode->GetNodeStatus() == BT::NodeStatus::RUNNING)
		{
			return BT::NodeStatus::RUNNING;
		}
		else if (childNode->GetNodeStatus() == BT::NodeStatus::FAILURE)
		{
			return BT::NodeStatus::FAILURE;
		}
	}

	ShuffleChildrenNodes();

	return BT::NodeStatus::SUCCESS;
}
