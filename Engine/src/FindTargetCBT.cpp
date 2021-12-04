#include "EnginePCH.h"
#include "FindTargetCBT.h"

BT::FindTargetCBT::FindTargetCBT(const std::string& name, Entity entity)
	:ActionNode(name),
	entity(entity)
{
}

BT::FindTargetCBT::~FindTargetCBT() = default;

BT::NodeStatus BT::FindTargetCBT::Tick()
{
	//Get all players in current game
	const PlayersPosition_t* playersEntity = Blackboard::Get().GetValue<PlayersPosition_t>("players");
	//Get AI's transform component
	const comp::Transform* transform = entity.GetComponent<comp::Transform>();

	if (playersEntity == nullptr)
	{
		LOG_ERROR("Failed to get players entity");
		return BT::NodeStatus::FAILURE;
	}

	if (transform == nullptr)
	{
		LOG_ERROR("The enemy have no transform");
		return BT::NodeStatus::FAILURE;
	}

	PathFinderManager* pathFinderManager = Blackboard::Get().GetPathFindManager();

	//Find the nearest player / villager and set that position as target
	Entity nearestPlayer;
	for (auto player : playersEntity->players)
	{
		comp::Player* playerComp = player.GetComponent<comp::Player>();
		comp::Villager* villagerComp = player.GetComponent<comp::Villager>();

		if (playerComp && player.GetComponent<comp::Player>()->reachable || villagerComp && !villagerComp->isHiding)
		{
			comp::Transform* currentNearest = nullptr;
			comp::Transform* playerTransform = player.GetComponent<comp::Transform>();
			comp::Health* playerHealth = player.GetComponent<comp::Health>();


			//If no nearest is assigned, assign someone who is alive
			if (nearestPlayer.IsNull() && playerHealth->isAlive)
			{
				nearestPlayer = player;
				continue;
			}

			//If we have a target assign transform component
			if (!nearestPlayer.IsNull())
			{
				currentNearest = nearestPlayer.GetComponent<comp::Transform>();
			}

			//Missing components
			if (!playerTransform || !playerHealth || !currentNearest)
				continue;

			Node* closestNode = pathFinderManager->FindClosestNode(playerTransform->position);

			//If can find a closer target that is alive, update to the closer target
			if (sm::Vector3::Distance(transform->position, playerTransform->position) < sm::Vector3::Distance(transform->position, currentNearest->position)
				&& closestNode->reachable && playerHealth->isAlive)
			{
				nearestPlayer = player;
			}
		}
	}

	//If target was set return success
	if (!nearestPlayer.IsNull())
	{
		Blackboard::Get().AddValue(("target" + std::to_string(entity)), nearestPlayer);
		return BT::NodeStatus::SUCCESS;

	}

	//If not try to find closest defense building to attack.
	std::unordered_map<Entity, Entity>* defenseEntities = Blackboard::Get().GetPathFindManager()->GetDefenseEntities();
	Entity currentTarget;
	//Check all defenses and take the closest one
	for (auto defenseEntity : *defenseEntities)
	{
		//If no target set the first one.
		if (currentTarget.IsNull())
		{
			currentTarget = defenseEntity.second;
			continue;
		}

		//If value in map is null remove it from the system
		if (defenseEntity.second.IsNull())
		{
			Blackboard::Get().GetPathFindManager()->RemoveDefenseEntity(defenseEntity.second);
			continue;
		}

		//Check if it's closer than current target
		if (sm::Vector3::Distance(transform->position, defenseEntity.second.GetComponent<comp::Transform>()->position) <
			sm::Vector3::Distance(transform->position, currentTarget.GetComponent<comp::Transform>()->position))
		{
			currentTarget = defenseEntity.second;
		}
	}
	//If target building was found return success
	if (!currentTarget.IsNull())
	{
		Blackboard::Get().AddValue(("target" + std::to_string(entity)), currentTarget);
		return BT::NodeStatus::SUCCESS;
	}

	//If no target was set return failure
	return BT::NodeStatus::FAILURE;
}
