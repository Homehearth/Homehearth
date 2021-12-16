#pragma once
#include <utility>

#include "LeafNodeBT.h"
namespace BT
{
	class ConditionNode : public LeafNode
	{
	public:
		ConditionNode(std::string name): LeafNode(std::move(name)){};
		~ConditionNode() override = default;
		virtual BT::NodeStatus Tick() = 0;
	};
}


