#include "EnginePCH.h"
#include "ParentNodeBT.h"

BT::ParentNode::ParentNode(std::string& name)
	:TreeNode(name), childStatus(NodeStatus::IDLE)
{
}

BT::ParentNode::~ParentNode()
{
}

void BT::ParentNode::AddChild(std::shared_ptr<BT::TreeNode> child)
{
	this->m_childrenNodes.emplace_back(child);
	this->m_childrenStates.emplace_back(BT::NodeStatus::IDLE);
}

int BT::ParentNode::GetNumChildren() const
{
	return (int)this->m_childrenNodes.size();
}

std::vector<std::shared_ptr<BT::TreeNode>> BT::ParentNode::GetChildren()
{
	return this->m_childrenNodes;
}
