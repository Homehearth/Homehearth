#include "NetServerPCH.h"
#include "Simulation.h"

#include "Wave.h"

void Simulation::InsertEntityIntoMessage(Entity entity, message<GameMsg>& msg, const std::bitset<ecs::Component::COMPONENT_MAX>& componentMask)const
{
	std::bitset<ecs::Component::COMPONENT_MAX> compSet;

	for (uint32_t i = 0; i < ecs::Component::COMPONENT_COUNT; i++)
	{
		if (!componentMask.test(i))
			continue;
		
		switch (i)
		{
		case ecs::Component::TRANSFORM:
		{
			comp::Transform* t = entity.GetComponent<comp::Transform>();
			if (t)
			{
				compSet.set(ecs::Component::TRANSFORM);
				msg << *t;
			}
			break;
		}
		case ecs::Component::VELOCITY:
		{
			comp::Velocity* v = entity.GetComponent<comp::Velocity>();
			if (v)
			{
				compSet.set(ecs::Component::VELOCITY);
				msg << *v;
			}
			break;
		}
		case ecs::Component::MESH_NAME:
		{
			comp::MeshName* m = entity.GetComponent<comp::MeshName>();
			if (m)
			{
				compSet.set(ecs::Component::MESH_NAME);
				msg << m->name;
			}

			break;
		}
		case ecs::Component::ANIMATOR_NAME:
		{
			comp::AnimatorName* m = entity.GetComponent<comp::AnimatorName>();
			if (m)
			{
				compSet.set(ecs::Component::ANIMATOR_NAME);
				msg << m->name;
			}

			break;
		}
		case ecs::Component::NAME_PLATE:
		{
			comp::NamePlate* n = entity.GetComponent<comp::NamePlate>();
			if (n)
			{
				compSet.set(ecs::Component::NAME_PLATE);
				msg << n->namePlate;
			}
			break;
		}
		case ecs::Component::HEALTH:
		{
			comp::Health* h = entity.GetComponent<comp::Health>();
			if (h)
			{
				compSet.set(ecs::Component::HEALTH);
				msg << *h;
			}
			break;
		}
		case ecs::Component::BOUNDING_ORIENTED_BOX:
		{
			comp::BoundingOrientedBox* b = entity.GetComponent<comp::BoundingOrientedBox>();
			if (b)
			{
				compSet.set(ecs::Component::BOUNDING_ORIENTED_BOX);
				msg << *b;
			}
			break;
		}
		case ecs::Component::BOUNDING_SPHERE:
		{
			comp::BoundingSphere* bs = entity.GetComponent<comp::BoundingSphere>();
			if (bs)
			{
				compSet.set(ecs::Component::BOUNDING_SPHERE);
				msg << *bs;
			}
			break;
		}
		case ecs::Component::LIGHT:
		{
			comp::Light* l = entity.GetComponent<comp::Light>();
			if (l)
			{
				compSet.set(ecs::Component::LIGHT);
				msg << *l;
			}
			break;
		}
		case ecs::Component::PLAYER:
		{
			comp::Player* p = entity.GetComponent<comp::Player>();
			if (p)
			{
				compSet.set(ecs::Component::PLAYER);
				msg << *p;
			}
			break;
		}
		default:
			LOG_WARNING("Trying to send unimplemented component %u", i);
			break;
		}
	}

	msg << static_cast<uint32_t>(compSet.to_ulong());
	msg << entity.GetComponent<comp::Network>()->id;
}

message<GameMsg> Simulation::AllEntitiesMessage()const
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_AddEntity;
	uint32_t count = 0;
	m_pGameScene->ForEachComponent<comp::Network>([&](Entity e, comp::Network& n)
		{
			InsertEntityIntoMessage(e, msg);
			count++;
		});
	msg << count;
	return msg;
}

void Simulation::CreateWaves()
{
	using namespace EnemyManagement;
	//Reeset wavequeu
	while (!waveQueue.empty())
	{
		waveQueue.pop();
	}

	//left bottom corner { 490.f, -150.0f });   right bottom corner { 170, -80.0f }
	//left top corner { 520.f, -540.0f }        right top corner { 80.0f, -500.0f }
	Wave wave1, wave2, wave3, wave4, wave5; // Default: WaveType::Zone
	{
		Wave::Group group1;
		group1.AddEnemy(EnemyType::Default,4 + 2 * currentRound);
		group1.SetSpawnPoint({ 490.f, -150.0f });
		wave1.SetTimeLimit(5 * currentRound);
		wave1.AddGroup(group1);
	}

	{ // Wave_2
		Wave::Group group1, group2;

		group1.AddEnemy(EnemyType::Default, 3 + currentRound);
		group2.AddEnemy(EnemyType::Default, 2 + currentRound);
		group2.AddEnemy(EnemyType::Runner, 1 + 2 * currentRound);
		group1.SetSpawnPoint({ 490.f, -150.0f });
		group2.SetSpawnPoint({ 170, -80.0f });
		wave2.AddGroup(group1);
		wave2.AddGroup(group2);
		wave2.SetTimeLimit(30);
	}


	{ // Wave_3
		Wave::Group group1, group2, group3, group4;

		group1.AddEnemy(EnemyType::Default, 3 + currentRound);
		group1.AddEnemy(EnemyType::Runner, 1 + 1 * currentRound);
		group1.SetSpawnPoint({ 490.f, -150.0f });

		group2.AddEnemy(EnemyType::Default, 3 + currentRound);
		group2.AddEnemy(EnemyType::Runner, 1 + currentRound);
		group2.SetSpawnPoint({ 170, -80.0f });

		group3.AddEnemy(EnemyType::Default, 3 + currentRound);
		group3.SetSpawnPoint({ 80.0f, -500.0f });

		group4.AddEnemy(EnemyType::Default, 2 + currentRound);
		group4.SetSpawnPoint({ 520.f, -540.0f });

		wave3.AddGroup(group1);
		wave3.AddGroup(group2);
		wave3.AddGroup(group3);
		wave3.AddGroup(group4);
		wave3.SetTimeLimit(45);
	}

	{ // Wave_4
		Wave::Group group1, group2, group3, group4;

		group1.AddEnemy(EnemyType::Default, 4 + currentRound);
		group1.AddEnemy(EnemyType::Runner, 1 + currentRound);
		group1.SetSpawnPoint({ 490.f, -150.0f });

		group2.AddEnemy(EnemyType::Default, 4 + currentRound);
		group2.AddEnemy(EnemyType::Runner, 1 + currentRound);
		group2.SetSpawnPoint({ 170, -80.0f });

		group3.AddEnemy(EnemyType::Default, 2 + currentRound);
		group3.AddEnemy(EnemyType::Runner, 2 + currentRound);
		group3.SetSpawnPoint({ 80.0f, -500.0f });

		group4.AddEnemy(EnemyType::Default, 4 + currentRound);
		group4.AddEnemy(EnemyType::Runner, 1 + currentRound);
		group4.SetSpawnPoint({ 520.f, -540.0f });

		wave4.AddGroup(group1);
		wave4.AddGroup(group2);
		wave4.AddGroup(group3);
		wave4.AddGroup(group4);
		wave4.SetTimeLimit(45);
	}

	{ // Wave_5 BOSS
		Wave::Group group1, group2, group3, group4;

		group1.AddEnemy(EnemyType::Default, 2 + currentRound);
		group1.AddEnemy(EnemyType::BIGMOMMA, 1);
		group1.SetSpawnPoint({ 490.f, -150.0f });

		group2.AddEnemy(EnemyType::Default, 1 + currentRound);
		group2.SetSpawnPoint({ 170, -80.0f });

		group3.AddEnemy(EnemyType::Default, 2 + currentRound);
		group3.SetSpawnPoint({ 80.0f, -500.0f });

		group4.AddEnemy(EnemyType::Default, 1 + currentRound);
		group4.SetSpawnPoint({ 520.f, -540.0f });

		wave5.AddGroup(group1);
		wave5.AddGroup(group2);
		wave5.AddGroup(group3);
		wave5.AddGroup(group4);
		wave5.SetTimeLimit(45);
	}

	waveQueue.emplace(wave1); 
	waveQueue.emplace(wave2); 
	waveQueue.emplace(wave3); 
	waveQueue.emplace(wave4); 
	waveQueue.emplace(wave5); 
}

void Simulation::ResetPlayer(Entity player)
{
	
	comp::Player* playerComp = player.GetComponent<comp::Player>();
	if (!playerComp)
	{
		LOG_WARNING("ResetPlayer: Entity is not a Player");
		return;
	}

	playerComp->runSpeed = 25.f;
	playerComp->state = comp::Player::State::IDLE;
	playerComp->isReady = false;

	comp::Transform* transform = player.AddComponent<comp::Transform>();

	transform->position = playerComp->spawnPoint;
	transform->scale = sm::Vector3(1.8f, 1.8f, 1.8f);

	player.AddComponent<comp::Velocity>();

	bool firstTimeAdded = false;
	if (!player.GetComponent<comp::AnimatorName>())
	{
		firstTimeAdded = true;
	}

	player.AddComponent<comp::MeshName>()->name = "Knight.fbx";
	player.AddComponent<comp::AnimatorName>()->name = "Knight.anim";

	
	comp::AttackAbility* attackAbility = player.AddComponent<comp::AttackAbility>();

	// only if Melee
	if (playerComp->classType == comp::Player::Class::WARRIOR)
	{
		attackAbility->cooldown = 0.3f;
		attackAbility->attackDamage = 40.f;
		attackAbility->isRanged = false;
		attackAbility->lifetime = 0.2f;
		attackAbility->useTime = 0.2f;
		attackAbility->delay = 0.1f;

		playerComp->primaryAbilty = entt::resolve<comp::AttackAbility>();
		playerComp->secondaryAbilty = entt::resolve<comp::AttackAbility>();

	}
	else if(playerComp->classType == comp::Player::Class::MAGE) 
	{
		attackAbility->cooldown = 0.5f;
		attackAbility->attackDamage = 20.f;
		attackAbility->isRanged = true;
		attackAbility->lifetime = 2.0f;
		attackAbility->projectileSpeed = 40.f;
		attackAbility->attackRange = 2.0f;
		attackAbility->useTime = 0.3f;
		attackAbility->delay = 0.1f;
		playerComp->primaryAbilty = entt::resolve<comp::AttackAbility>();

		comp::HealAbility* healAbility = player.AddComponent<comp::HealAbility>();
		healAbility->cooldown = 5.0f;
		healAbility->delay = 0.0f;
		healAbility->healAmount = 50.f;
		healAbility->lifetime = 1.f;
		healAbility->range = 50.f;
		healAbility->useTime = 1.0f;

		playerComp->secondaryAbilty = entt::resolve<comp::HealAbility>();


		player.AddComponent<comp::MeshName>()->name = "Monster.fbx";
		player.AddComponent<comp::AnimatorName>()->name = "Monster.anim";

	}

	comp::Health* health = player.AddComponent<comp::Health>();
	health->currentHealth = 100.f;
	health->isAlive = true;


	//player.AddComponent<comp::BoundingOrientedBox>()->Extents = { 2.0f,2.0f,2.0f };
	player.AddComponent<comp::BoundingSphere>()->Radius = 3.f;

	//Collision will handle this entity as a dynamic one
	player.AddComponent<comp::Tag<TagType::DYNAMIC>>();
	player.AddComponent<comp::Tag<TagType::GOOD>>(); // this is a good guy, he will call you back

	player.RemoveComponent<comp::TemporaryPhysics>();
	if (!firstTimeAdded)
	{
		m_pGameScene->publish<EComponentUpdated>(player, ecs::Component::MESH_NAME);
		m_pGameScene->publish<EComponentUpdated>(player, ecs::Component::ANIMATOR_NAME);
	}

	m_pGameScene->publish<EComponentUpdated>(player, ecs::Component::HEALTH);
	player.UpdateNetwork();
	
}

Simulation::Simulation(Server* pServer, HeadlessEngine* pEngine)
	: m_pServer(pServer)
	, m_pEngine(pEngine), m_pLobbyScene(nullptr), m_pGameScene(nullptr), m_pCurrentScene(nullptr), currentRound(0)
{
	this->m_gameID = 0;
	this->m_tick = 0;
}

bool Simulation::JoinLobby(uint32_t playerID, uint32_t gameID, const std::string& namePlate)
{
	if (m_pCurrentScene == m_pGameScene)
	{
		message<GameMsg> msg;
		msg.header.id = GameMsg::Lobby_Invalid;
		msg << std::string("Request denied: Game has already started!");

		m_pServer->SendToClient(playerID, msg);

		return false;
	}
	// Send to client the message with the new game ID
	if (m_players.size() < MAX_PLAYERS_PER_LOBBY)
	{
		message<GameMsg> msg;
		msg.header.id = GameMsg::Lobby_Accepted;
		msg << gameID;

		m_pServer->SendToClient(playerID, msg);

		// Add the players to the simulation on that specific client
		this->AddPlayer(playerID, namePlate);
	}
	else
	{
		network::message<GameMsg> msg;
		msg.header.id = GameMsg::Lobby_Invalid;
		std::string fullLobby = "Request denied: This lobby was full!";
		msg << fullLobby;
		m_pServer->SendToClient(playerID, msg);

		return false;
	}

	LOG_INFO("Player %d joined lobby: %d", playerID, gameID);
	return true;
}

bool Simulation::LeaveLobby(uint32_t playerID, uint32_t gameID)
{
	if (!this->RemovePlayer(playerID))
	{
		LOG_INFO("Could not remove player!");
		return false;
	}

	// Send to client the message with the new game ID
	message<GameMsg> accMsg;
	accMsg.header.id = GameMsg::Lobby_AcceptedLeave;

	m_pServer->SendToClient(playerID, accMsg);

	return true;
}

void Simulation::UpdateLobby()
{
	network::message<GameMsg> msg;
	msg.header.id = GameMsg::Lobby_Update;

	for (auto& player : m_players)
	{
		// Send ID first.
		msg << (uint32_t)player.first;
		// After that name.
		msg << player.second.GetComponent<comp::NamePlate>()->namePlate;
	}
	msg << (uint32_t)m_players.size();

	// Send message to each other player and skip the current one.
	this->Broadcast(msg);
}

bool Simulation::Create(uint32_t playerID, uint32_t gameID, std::vector<dx::BoundingOrientedBox>* mapColliders, const std::string& namePlate)
{
	this->m_gameID = gameID;

	//Set players spawn point
	m_spawnPoints.push(sm::Vector3(220.f, 0, -353.f));
	m_spawnPoints.push(sm::Vector3(197.f, 0, -325.f));
	m_spawnPoints.push(sm::Vector3(222.f, 0, -300.f));
	m_spawnPoints.push(sm::Vector3(247.f, 0, -325.f));

	// Create and add all waves to the queue.
	CreateWaves();

	// Create Scenes associated with this Simulation
	m_pLobbyScene = &m_pEngine->GetScene("Lobby_" + std::to_string(gameID));
	m_pLobbyScene->on<ESceneUpdate>([&](const ESceneUpdate& e, HeadlessScene& scene)
		{
			//LOG_INFO("LOBBY Scene %d", gameID);

		});

	m_pGameScene = &m_pEngine->GetScene("Game_" + std::to_string(gameID));
	m_pGameScene->on<ESceneUpdate>([&](const ESceneUpdate& e, HeadlessScene& scene)
		{
			// update components with input
			for (const auto& pair : m_playerInputs)
			{
				Entity e = pair.first;
				comp::Player* p = e.GetComponent<comp::Player>();
				comp::Health* h = e.GetComponent<comp::Health>();
				if (h->isAlive)
				{
					InputState input = pair.second;
					p->lastInputState = input;
				}
			}

			//  run all game logic systems
			{
				PROFILE_SCOPE("Systems");
				
				ServerSystems::CheckGameOver(this, scene);

				AIBehaviors::UpdateBlackBoard(scene);
				
				ServerSystems::TickBTSystem(this, scene);
				ServerSystems::UpdatePlayerWithInput(this, scene, e.dt);
				ServerSystems::PlayerStateSystem(this, scene, e.dt);
				
				Systems::UpdateAbilities(scene, e.dt);
				Systems::CombatSystem(scene, e.dt);
				Systems::HealingSystem(scene, e.dt);

				Systems::HealthSystem(scene, e.dt, m_currency.GetAmountRef());
				Systems::SelfDestructSystem(scene, e.dt);
				
				{
					PROFILE_SCOPE("Collision Box/Box");
					//Systems::CheckCollisions<comp::BoundingOrientedBox, comp::BoundingOrientedBox>(scene, e.dt);
				}
				{
					PROFILE_SCOPE("Collision Sphere/Box");
					Systems::CheckCollisions<comp::BoundingSphere, comp::BoundingOrientedBox>(scene, e.dt);
				}
				{
					PROFILE_SCOPE("Collision Sphere/Sphere");
					Systems::CheckCollisions<comp::BoundingSphere, comp::BoundingSphere>(scene, e.dt);
				}
				
				Systems::MovementSystem(scene, e.dt);
				Systems::MovementColliderSystem(scene, e.dt);
			}

			if (!waveQueue.empty())
				ServerSystems::NextWaveConditions(this, waveTimer, waveQueue.front().GetTimeLimit());
			else
				this->CreateWaves();
			//LOG_INFO("GAME Scene %d", m_gameID);
		});

	//On all enemies wiped, activate the next wave.
	m_pGameScene->on<ESceneCallWaveSystem>([&](const ESceneCallWaveSystem& dt, HeadlessScene& scene)
		{
			waveTimer.Start();
			ServerSystems::WaveSystem(this, waveQueue);

		});


	m_pGameScene->GetRegistry()->on_construct<comp::Network>().connect<&Simulation::OnNetworkEntityCreate>(this);
	m_pGameScene->GetRegistry()->on_destroy<comp::Network>().connect<&Simulation::OnNetworkEntityDestroy>(this);
	m_pGameScene->GetRegistry()->on_update<comp::Network>().connect<&Simulation::OnNetworkEntityUpdated>(this);
	m_pGameScene->on<EComponentUpdated>([&](const EComponentUpdated& e, HeadlessScene& scene)
		{
			OnComponentUpdated(e.entity, e.component);
		});

	//Gridsystem
	m_grid.Initialize(gridOptions.mapSize, gridOptions.position, gridOptions.fileName, m_pGameScene);
	//Create the nodes for AI handler on blackboard
	Blackboard::Get().GetAIHandler()->CreateNodes(&m_grid);

#if RENDER_AINODES
	std::vector<std::vector<std::shared_ptr<Node>>> nodes = m_aiHandler.GetNodes();
	for(int y = 0; y < nodes[0].size(); y++)
	{
		for (int x = 0; x < nodes[0].size(); x++)
		{
			if(nodes[y][x].get()->reachable)
			{
				Entity cube = m_pGameScene->CreateEntity();
				cube.AddComponent<comp::Transform>()->position = nodes[y][x].get()->position;
				cube.AddComponent<comp::MeshName>()->name = "Cube.obj";
				cube.AddComponent<comp::Network>();
			}

		}
	}
#endif RENDER_AINODES
	m_addedEntities.clear();
	m_removedEntities.clear();


	this->BuildMapColliders(mapColliders);

	m_pCurrentScene = m_pLobbyScene;

	// Automatically join created lobby
	JoinLobby(playerID, gameID, namePlate);


	return true;
}

void Simulation::Destroy()
{
	m_pGameScene->GetRegistry()->on_construct<comp::Network>().disconnect<&Simulation::OnNetworkEntityCreate>(this);
	m_pGameScene->GetRegistry()->on_destroy<comp::Network>().disconnect<&Simulation::OnNetworkEntityDestroy>(this);
	m_pGameScene->Clear();
	m_pLobbyScene->Clear();
}

void Simulation::ReadyCheck(const uint32_t& playerID)
{
	if (m_pCurrentScene == m_pLobbyScene)
	{
		if (m_players.find(playerID) != m_players.end())
		{
			comp::Player* player = m_players.at(playerID).GetComponent<comp::Player>();
			player->isReady = !player->isReady;

			// DEBUG
#ifdef _DEBUG
			// Debugging allow only one player to start.
			//m_pCurrentScene = m_pGameScene;
			//// Start the game.
			//network::message<GameMsg> msg;
			//msg.header.id = GameMsg::Game_Start;
			//this->Broadcast(msg);
#endif

			auto it = m_players.begin();

			uint32_t readyCount = 0;
			while (it != m_players.end())
			{
				if (it->second.GetComponent<comp::Player>()->isReady)
				{
					readyCount++;
				}
				it++;
			}

			// Start game when all players are marked ready
			if (readyCount == m_players.size())
			{
				SetGameScene();
				// Start the game.
				network::message<GameMsg> msg;
				msg.header.id = GameMsg::Game_Start;
				this->Broadcast(msg);
			}
		}
	}
}

bool Simulation::IsEmpty() const
{
	return m_players.empty();
}


bool Simulation::AddPlayer(uint32_t playerID, const std::string& namePlate)
{
	if (!m_pServer->isClientConnected(playerID))
	{
		return false;
	}

	// Send all entities in Game Scene to new player
	this->SendAllEntitiesToPlayer(playerID);

	// Create Player entity in Game scene
	Entity player = m_pGameScene->CreateEntity();
	
	comp::Player* playerComp = player.AddComponent<comp::Player>();
	playerComp->spawnPoint = m_spawnPoints.front();
	m_spawnPoints.pop();
	player.AddComponent<comp::Network>(playerID);
	player.AddComponent<comp::NamePlate>()->namePlate = namePlate;
	
	ResetPlayer(player);

	m_players[playerID] = player;

	return true;
}

bool Simulation::RemovePlayer(uint32_t playerID)
{
	this->SendRemoveAllEntitiesToPlayer(playerID);

	Entity player = m_players.at(playerID);
	if (m_playerInputs.find(player) != m_playerInputs.end())
	{
		m_playerInputs.erase(player);
	}
	m_spawnPoints.push(player.GetComponent<comp::Player>()->spawnPoint);
	m_players.erase(playerID);

	if (!player.Destroy())
	{
		LOG_INFO("Player %u entity could not be removed", playerID);
		return false;
	}
	LOG_INFO("Removed player %u from scene", playerID);

	return true;
}

std::unordered_map<uint32_t, Entity>::iterator Simulation::RemovePlayer(std::unordered_map<uint32_t, Entity>::iterator playerIterator)
{
	Entity player = playerIterator->second;
	uint32_t playerID = playerIterator->first;
	if (m_playerInputs.find(player) != m_playerInputs.end())
	{
		m_playerInputs.erase(player);
	}
	m_spawnPoints.push(player.GetComponent<comp::Player>()->spawnPoint);
	auto it = m_players.erase(playerIterator);

	if (!player.Destroy())
	{
		LOG_INFO("Player %u entity could not be removed", playerID);
	}
	else
	{
		LOG_INFO("Removed player %u from scene", playerID);
	}

	return it;
}

void Simulation::SendSnapshot()
{
	PROFILE_FUNCTION();
	// remove any client disconnected
	this->ScanForDisconnects();

	// all new Entities
	this->SendEntities(m_addedEntities, GameMsg::Game_AddEntity);
	m_addedEntities.clear();


	if (m_pCurrentScene == m_pGameScene)
	{
		if (!m_updatedComponents.empty())
		{
			for (auto& pair : m_updatedComponents)
			{
				std::bitset<ecs::Component::COMPONENT_MAX> compMask;
				compMask.set(pair.first);
				this->SendEntities(pair.second, GameMsg::Game_UpdateComponent, compMask);
			}
			m_updatedComponents.clear();
		}

		std::bitset<ecs::Component::COMPONENT_MAX> compMask;
		compMask.set(ecs::Component::TRANSFORM);
#if DEBUG_SNAPSHOT
		compMask.set(ecs::Component::BOUNDING_ORIENTED_BOX);
		compMask.set(ecs::Component::BOUNDING_SPHERE);
#endif

		this->SendEntities(m_updatedEntities, GameMsg::Game_Snapshot, compMask);
		m_updatedEntities.clear();

		// Update until next wave timer if next wave is present.
		if (!waveQueue.empty())
		{
			// Update wave timer to clients.
			network::message<GameMsg> msg2;
			msg2.header.id = GameMsg::Game_WaveTimer;
			uint32_t timer = (uint32_t)waveQueue.front().GetTimeLimit() - (uint32_t)waveTimer.GetElapsedTime<std::chrono::seconds>();
			msg2 << timer;
			this->Broadcast(msg2);
		}
	}
	else if (m_pCurrentScene == m_pLobbyScene)
	{
		this->UpdateLobby();
	}

	// all destroyed Entities
	this->SendRemoveEntities(m_removedEntities);
	m_removedEntities.clear();

	network::message<GameMsg> msg3;
	msg3.header.id = GameMsg::Game_Money;
	msg3 << m_currency.GetAmount();
	this->Broadcast(msg3);
}

void Simulation::Update(float dt)
{
	PROFILE_FUNCTION();
	if (m_pCurrentScene)
		m_pCurrentScene->Update(dt);
}

void Simulation::UpdateInput(InputState state, uint32_t playerID)
{
	if (m_pCurrentScene != m_pGameScene)
		return;

	if (m_players.find(playerID) != m_players.end())
	{
		m_playerInputs[m_players.at(playerID)] = state;
	}
}

void Simulation::NextTick()
{
	this->m_tick++;
}

uint32_t Simulation::GetTick() const
{
	return this->m_tick;
}

void Simulation::Broadcast(network::message<GameMsg>& msg, uint32_t exclude)const
{
	auto it = m_players.begin();

	while (it != m_players.end())
	{
		if (exclude != it->first)
		{
			m_pServer->SendToClient(it->first, msg);
		}
		it++;
	}
}

void Simulation::BroadcastUDP(message<GameMsg>& msg, uint32_t exclude) const
{
	auto it = m_players.begin();

	while (it != m_players.end())
	{
		if (exclude != it->first)
		{
			m_pServer->SendToClientUDP(it->first, msg);
		}
		it++;
	}
}

void Simulation::ScanForDisconnects()
{
	auto it = m_players.begin();

	while (it != m_players.end())
	{
		if (!m_pServer->isClientConnected(it->first))
		{
			it = this->RemovePlayer(it);
		}
		else
		{
			it++;
		}
	}
}

void Simulation::OnNetworkEntityCreate(entt::registry& reg, entt::entity entity)
{
	Entity e(reg, entity);
	// Network has surely been added
	comp::Network* net = e.GetComponent<comp::Network>();
	if (net->id == UINT32_MAX)
	{
		net->id = m_pServer->PopNextUniqueID();
	}
	m_addedEntities.push_back(e);
}

void Simulation::OnNetworkEntityDestroy(entt::registry& reg, entt::entity entity)
{
	Entity e(reg, entity);
	// Network has not been destroyed yet
	comp::Network* net = e.GetComponent<comp::Network>();
	m_removedEntities.push_back(net->id);
	auto it = std::find(m_updatedEntities.begin(), m_updatedEntities.end(), e);
	if (it != m_updatedEntities.end())
	{
		m_updatedEntities.erase(it);
	}
}

void Simulation::OnNetworkEntityUpdated(entt::registry& reg, entt::entity entity)
{
	Entity e(reg, entity);
	if (std::find(m_updatedEntities.begin(), m_updatedEntities.end(), e) == m_updatedEntities.end())
	{
		m_updatedEntities.push_back(e);
	}
}
void Simulation::OnComponentUpdated(Entity entity, ecs::Component component)
{
	if (entity.GetComponent<comp::Network>())
	{
		if (std::find(m_updatedComponents[component].begin(), m_updatedComponents[component].end(), entity) == m_updatedComponents[component].end())
		{
			m_updatedComponents[component].push_back(entity);
		}
	}
}



void Simulation::BuildMapColliders(std::vector<dx::BoundingOrientedBox>* mapColliders)
{
	// --- END OF THE WORLD ---
	Entity collider;
	for (size_t i = 0; i < mapColliders->size(); i++)
	{
		collider = m_pGameScene->CreateEntity();
		dx::BoundingOrientedBox* obb = collider.AddComponent<comp::BoundingOrientedBox>();
		obb->Center = mapColliders->at(i).Center;
		obb->Extents = mapColliders->at(i).Extents;
		obb->Orientation = mapColliders->at(i).Orientation;
		collider.AddComponent<comp::Tag<TagType::STATIC>>();
		//collider.AddComponent<comp::Network>();
	}
}

HeadlessScene* Simulation::GetLobbyScene() const
{
	return m_pLobbyScene;
}

HeadlessScene* Simulation::GetGameScene() const
{
	return m_pGameScene;
}

Entity* Simulation::GetPlayer(uint32_t entityID)
{
	if (m_players.find(entityID) == m_players.end())
	{
		return nullptr;
	}

	return &m_players[entityID];
}

GridSystem& Simulation::GetGrid()
{
	return m_grid;
}

Currency& Simulation::GetCurrency()
{
	return m_currency;
}

void Simulation::UseShop(const ShopItem& item, const uint32_t& player)
{
	switch (item)
	{
	case ShopItem::Primary_Upgrade:
	{
		comp::AttackAbility* p = m_players.at(player).GetComponent<comp::AttackAbility>();
		if (p && m_currency.GetAmountRef() >= 10)
		{
			p->attackDamage += 2.0f;
			m_currency.GetAmountRef() -= 10;
		}
		break;
	}
	case ShopItem::Tower_Upgrade:
	{
		/*Upgrade a tower or ALL towers?*/
		break;
	}
	default:
	{
		break;
	}
	}
}

void Simulation::SetLobbyScene()
{
	m_pCurrentScene = m_pLobbyScene;
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_BackToLobby;

	Broadcast(msg);
}

void Simulation::SetGameScene()
{
	ResetGameScene();
	m_pCurrentScene = m_pGameScene;
#ifdef _DEBUG
	// During debug give players 1000 gold/monies.
	m_currency.GetAmountRef() = 1000;

#endif
}

void Simulation::ResetGameScene()
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_RemoveEntity;
	uint32_t count = 0;
	currentRound = 0;
	this->m_pGameScene->ForEachComponent<comp::Network>([&](Entity e, comp::Network& n)
		{
			if (m_players.find(n.id) == m_players.end())
			{
				msg << n.id;
				count++;
				e.Destroy();
			}
			else
			{
				ResetPlayer(e);
			}
		});

	if (count > 0)
	{
		msg << count;
		Broadcast(msg);
	}

	m_currency.Zero();

	LOG_INFO("%lld", m_pGameScene->GetRegistry()->size());
	CreateWaves();
}

void Simulation::SendEntity(Entity e, const std::bitset<ecs::Component::COMPONENT_MAX>& componentMask)const
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_AddEntity;

	InsertEntityIntoMessage(e, msg, componentMask);

	msg << 1U;
	msg << GetTick();

	this->Broadcast(msg);
}

void Simulation::SendEntities(const std::vector<Entity>& entities, GameMsg msgID, const std::bitset<ecs::Component::COMPONENT_MAX>& componentMask) const
{
	if (entities.size() == 0)
		return;

	const size_t PACKET_CHUNK_SIZE = 10;


	uint32_t sent = 0;
	message<GameMsg> msg;
	msg.header.id = msgID;

	for (size_t i = 0; i < entities.size(); i++)
	{
		if (!entities[i].IsNull())
		{
			InsertEntityIntoMessage(entities[i], msg, componentMask);
			sent++;
			if (sent == PACKET_CHUNK_SIZE || i == entities.size() - 1)
			{
				msg << sent;
				msg << GetTick();

				if (msgID == GameMsg::Game_Snapshot)
				{
					BroadcastUDP(msg);
				}
				else {
					Broadcast(msg);
				}
				msg.clear();
				sent = 0;
				msg.header.id = msgID;
			}
		}
	}
}

void Simulation::SendAllEntitiesToPlayer(uint32_t playerID) const
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_AddEntity;
	uint32_t count = 0;
	m_pGameScene->ForEachComponent<comp::Network>([&](Entity e, comp::Network& n)
		{
			InsertEntityIntoMessage(e, msg);
			count++;
		});
	msg << count;
	msg << GetTick();

	this->m_pServer->SendToClient(playerID, msg);
}

void Simulation::SendRemoveAllEntitiesToPlayer(uint32_t playerID) const
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_RemoveEntity;

	uint32_t count = 0;
	m_pGameScene->ForEachComponent<comp::Network>([&](comp::Network& n)
		{
			msg << n.id;
			count++;
		});
	msg << count;

	m_pServer->SendToClient(playerID, msg);
}

void Simulation::SendRemoveSingleEntity(Entity e) const
{
	comp::Network* net = e.GetComponent<comp::Network>();
	if (net)
	{
		this->SendRemoveSingleEntity(net->id);
	}
	else
	{
		LOG_WARNING("Tried to remove entity without network component");
	}
}

void Simulation::SendRemoveSingleEntity(uint32_t networkID) const
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_RemoveEntity;

	msg << (uint32_t)networkID << 1U;

	this->Broadcast(msg);
}

void Simulation::SendRemoveEntities(message<GameMsg>& msg)const
{
	this->Broadcast(msg);
}

void Simulation::SendRemoveEntities(const std::vector<uint32_t> entitiesNetIDs) const
{
	if (entitiesNetIDs.size() == 0)
		return;

	message<GameMsg> msg;
	msg.header.id = GameMsg::Game_RemoveEntity;

	for (uint32_t e : entitiesNetIDs)
	{
		msg << e;
	}
	msg << static_cast<uint32_t>(entitiesNetIDs.size());

	this->Broadcast(msg);
}
