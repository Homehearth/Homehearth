#pragma once
#include "ParentNodeBT.h"

namespace BT
{
	class FallbackNode : public ParentNode
	{
	public:
		FallbackNode(std::string name);
		~FallbackNode() override = default;

		BT::NodeStatus Tick() override;
	};
}


