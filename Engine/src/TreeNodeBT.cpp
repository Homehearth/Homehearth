#include "EnginePCH.h"
#include "TreeNodeBT.h"


BT::TreeNode::TreeNode(std::string& name) :
	m_name(name),
	m_status(BT::NodeStatus::IDLE)
{
}

BT::NodeStatus BT::TreeNode::GetNodeStatus() const
{
	return m_status;
}

std::string BT::TreeNode::GetName() const
{
	return this->m_name;
}

void BT::TreeNode::SetNodeStatus(const NodeStatus nodeStatus)
{
	this->m_status = nodeStatus;
}

void BT::TreeNode::SetName(const std::string& name)
{
	this->m_name = name;
}
