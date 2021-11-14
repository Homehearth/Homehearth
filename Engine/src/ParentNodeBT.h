#pragma once
#include <vector>
#include "TreeNodeBT.h"
namespace BT
{
	class ParentNode : public TreeNode
	{
	public:
		ParentNode(std::string& name);
		~ParentNode() override;

		void AddChild(TreeNode* child);
		int GetNumChildren() const;
		std::vector<TreeNode*> GetChildren();

	protected:
		std::vector<TreeNode*> m_childrenNodes;
		std::vector<NodeStatus> m_childrenStates;
		NodeStatus childStatus;
	private:

	};
}


