#pragma once
#include <string>
#include <utility>

#include "LeafNodeBT.h"

namespace BT
{
	class ActionNode : public LeafNode
	{
	public:
		ActionNode(std::string name):LeafNode(std::move(name)){};
		~ActionNode() override = default;

		BT::NodeStatus Tick() override = 0;
	};
}


