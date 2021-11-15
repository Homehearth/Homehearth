#pragma once
#include <vector>
#include "TreeNodeBT.h"
namespace BT
{
	class FallbackNode;

	class ParentNode : public TreeNode
	{
	public:
		ParentNode(std::string& name);
		~ParentNode() override;

		void AddChild(std::shared_ptr<BT::TreeNode> child);
		int GetNumChildren() const;
		std::vector<std::shared_ptr<BT::TreeNode>> GetChildren();

	protected:
		std::vector<std::shared_ptr<TreeNode>> m_childrenNodes;
		std::vector<NodeStatus> m_childrenStates;
		NodeStatus childStatus;
	private:

	};
}


