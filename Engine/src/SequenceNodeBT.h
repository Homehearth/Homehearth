#pragma once
#include "ParentNodeBT.h"
namespace BT
{
	class SequenceNode : public ParentNode
	{
	public:
		SequenceNode(std::string name);
		~SequenceNode() override;

		BT::NodeStatus Tick() override;
	private:
	};
}


