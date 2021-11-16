#pragma once
#include "ParentNodeBT.h"
namespace BT
{
	class NonDeterministicNodeBT : public ParentNode
	{
	public:
		void ShuffleChildrenNodes();
		NonDeterministicNodeBT(std::string name);
		~NonDeterministicNodeBT() override = default;

		BT::NodeStatus Tick() override;
	private:
	};
}


