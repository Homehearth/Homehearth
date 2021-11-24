#include "EnginePCH.h"
#include "TeleportNodeCBT.h"

BT::TeleportNodeCBT::TeleportNodeCBT(const std::string& name, Entity entity)
	:ActionNode(name),
	entity(entity)
{
}

BT::TeleportNodeCBT::~TeleportNodeCBT() = default;

BT::NodeStatus BT::TeleportNodeCBT::Tick()
{
	comp::TeleportAbility* teleportAbility = entity.GetComponent<comp::TeleportAbility>();
	comp::Transform* transform = entity.GetComponent<comp::Transform>();
	PathFinderManager * pathFinderManager = Blackboard::Get().GetPathFindManager();
	Entity* currentTarget = Blackboard::Get().GetValue<Entity>("target" + std::to_string(entity));
	if(!teleportAbility)
	{
		LOG_WARNING("Attempt to use teleport ability without the component");
		return BT::NodeStatus::FAILURE;
	}
	if(!pathFinderManager)
	{
		LOG_WARNING("Blackboards pointer to pathFinderManager was null...");
		return BT::NodeStatus::FAILURE;
	}
	if(currentTarget == nullptr)
	{
		return BT::NodeStatus::FAILURE;
	}

	//Away from target
	sm::Vector3 teleportDirection = currentTarget->GetComponent<comp::Transform>()->position - transform->position;
	teleportDirection.Normalize();
	teleportDirection *= teleportAbility->distance;
	sm::Vector3 newPosition = transform->position + teleportDirection;

	sm::Quaternion rotation1 = sm::Quaternion::CreateFromAxisAngle(sm::Vector3(0.0f, 1.0f, 0.0f), 1.f);
	sm::Quaternion rotation2 = sm::Quaternion::CreateFromAxisAngle(sm::Vector3(0.0f, 1.0f, 0.0f), -1.f);

	teleportDirection= teleportDirection* rotation1;

	std::vector<sm::Quaternion> differentRotations = {
		{0.0f,0.0f,0.0f,0.0f},
		sm::Quaternion::CreateFromAxisAngle(sm::Vector3(0.0f, 1.0f, 0.0f), 1.f)
		,sm::Quaternion::CreateFromAxisAngle(sm::Vector3(0.0f, 1.0f, 0.0f), -1.f) };


	constexpr int attempts = 4;
	for (auto rotation : differentRotations)
	{
		sm::Vector3 tempTeleportDir = teleportDirection * rotation;

		Node* node = pathFinderManager->FindClosestNode(newPosition + tempTeleportDir);

		if(node->reachable)
		{
			teleportAbility->targetPoint = node->position;
			return BT::NodeStatus::SUCCESS;
		}

	}

	return BT::NodeStatus::FAILURE;
}
