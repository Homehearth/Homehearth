#include "EnginePCH.h"
#include "FallbackNodeBT.h"

BT::FallbackNode::FallbackNode(std::string name)
	:ParentNode(name)
{}

BT::NodeStatus BT::FallbackNode::Tick()
{
	for (const auto childNode : m_childrenNodes)
	{
		if (childNode->GetNodeStatus() == BT::NodeStatus::IDLE)
		{
			childStatus = childNode->Tick();
			childNode->SetNodeStatus(BT::NodeStatus::IDLE);

			//If one child Succeed, the whole node succeed.
			if (childStatus == BT::NodeStatus::SUCCESS)
			{
				return BT::NodeStatus::SUCCESS;
			}
		}
	}

	return BT::NodeStatus::FAILURE;

}
