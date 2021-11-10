#pragma once
#include "ParentNodeBT.h"

namespace BT
{
	class FallbackNode : public ParentNode
	{
		FallbackNode(std::string name);
		~FallbackNode() override = default;

		BT::NodeStatus Tick() override;
	};
}


