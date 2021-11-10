#include "EnginePCH.h"
#include "SequenceNodeBT.h"

BT::SequenceNode::SequenceNode(std::string name)
	:ParentNode(name)
{
}

BT::SequenceNode::~SequenceNode()
= default;

BT::NodeStatus BT::SequenceNode::Tick()
{
	for (const auto childNode : m_childrenNodes)
	{
		if(childNode->GetNodeStatus() == BT::NodeStatus::IDLE)
		{
			childStatus = childNode->Tick();
			childNode->SetNodeStatus(BT::NodeStatus::IDLE);

			//If one child fails, the whole node fails.
			if(childStatus == BT::NodeStatus::FAILURE)
			{
				return BT::NodeStatus::FAILURE;
			}
		}
	}

	return BT::NodeStatus::SUCCESS;
}
