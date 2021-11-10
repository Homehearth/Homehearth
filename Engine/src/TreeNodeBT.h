#pragma once
#include <string>

namespace BT
{
	// Enumerates the states every node can be in after execution during a particular time step.
	// the custom nodes should NEVER return IDLE.
	enum class NodeStatus
	{
	    IDLE = 0,
	    RUNNING,
	    SUCCESS,
	    FAILURE
	};

	enum class NodeType
	{
		ROOT,
		SEQUENCE,
		FALLBACK,
		CONDITION,
		ACTION
	};

	//an abstract base class for behavior tree nodes
	class TreeNode
	{
	public:
		TreeNode(std::string& name);
		virtual ~TreeNode() = default;

		//Function that are executed when reciving a tick
		virtual BT::NodeStatus Tick() = 0;

		//Get the current node status
		NodeStatus GetNodeStatus() const;
		std::string GetName() const;

		//Set the current node status
		void SetNodeStatus(NodeStatus nodeStatus);
		void SetName(const std::string& name);
	protected:
		NodeStatus m_status;
		NodeType type;
	private:
		//Nodes m_name
		std::string m_name;
	};

}

