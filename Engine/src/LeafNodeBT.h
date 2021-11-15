#pragma once
#include "TreeNodeBT.h"
namespace BT
{
	class LeafNode : public TreeNode
	{
	public:
		LeafNode(std::string name) : TreeNode(name){};
		~LeafNode() override = default;
	};
}


