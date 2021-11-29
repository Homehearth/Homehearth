#include "Game.h"
#include "GameSystems.h"
#include "Button.h"
#include "TextField.h"
#include "SceneHelper.h"
#include "Healthbar.h"
#include "MoneyUI.h"
#include "OptionSystem.h"

using namespace std::placeholders;

Game::Game()
	: m_client(std::bind(&Game::CheckIncoming, this, _1), std::bind(&Game::OnClientDisconnect, this))
	, Engine()
{
	this->m_localPID = -1;
	this->m_spectatingID = -1;
	this->m_money = 0;
	this->m_gameID = -1;
	this->m_waveTimer = 0;
}

Game::~Game()
{
	if (m_client.IsConnected())
	{
		m_client.Disconnect();
	}
}

void Game::UpdateNetwork(float deltaTime)
{
	static float pingCheck = 0.f;
	const float TARGET_PING_TIME = 5.0f;
	if (m_client.IsConnected())
	{
		m_client.Update();

		pingCheck += deltaTime;

		if (pingCheck > TARGET_PING_TIME)
		{
			this->PingServer();
			pingCheck -= TARGET_PING_TIME;
		}

		if (GetCurrentScene() == &GetScene("Game"))
		{
			if (GetCurrentScene()->GetCurrentCamera()->GetCameraType() == CAMERATYPE::PLAY)
			{
				message<GameMsg> msg;
				msg.header.id = GameMsg::Game_PlayerInput;

				msg << this->m_localPID << m_gameID << m_inputState;

				m_client.Send(msg);

				//reset input
				memset(&m_inputState, 0, sizeof(m_inputState));
			}
		}
	}
}

bool Game::OnStartup()
{
	m_masterVolume = std::stof(OptionSystem::Get().GetOption("MasterVolume"));
	sceneHelp::CreateLoadingScene(this);
	SetScene("Loading");

	// Scene logic
	sceneHelp::CreateLobbyScene(this);
	sceneHelp::CreateGameScene(this);
	sceneHelp::CreateMainMenuScene(this);
	sceneHelp::CreateJoinLobbyScene(this);
	sceneHelp::CreateGameOverScene(this);

	sceneHelp::CreateOptionsScene(this);
	// Set Current Scene
	SetScene("MainMenu");

	//Particles
	Entity emitter4 = GetScene("Game").CreateEntity();
	emitter4.AddComponent<comp::Transform>()->position = { 250, 5, -340 };
	emitter4.AddComponent <comp::EmitterParticle>(sm::Vector3{ 0,0,0 }, 800, 2.f, PARTICLEMODE::SMOKE, 4.0f, 1.f, false);

	Entity waterSplash = GetScene("Game").CreateEntity();
	waterSplash.AddComponent<comp::Transform>()->position = { 270, 13, -370 };
	waterSplash.AddComponent <comp::EmitterParticle>(sm::Vector3{ 0,0,0 }, 100, 1.f , PARTICLEMODE::WATERSPLASH, 4.0f, 1.f, false);

	return true;
}

void Game::OnUserUpdate(float deltaTime)
{
	this->UpdateInput();
	Scene& scene = GetScene("Game");
	if (m_players.find(m_localPID) != m_players.end())
	{
		sm::Vector3 playerPos = m_players.at(m_localPID).GetComponent<comp::Transform>()->position;

		GameSystems::DeathParticleTimer(scene);

		if (m_elapsedCycleTime <= m_waveTimer && (m_serverCycle == Cycle::DAY || m_serverCycle == Cycle::MORNING))
		{
			m_elapsedCycleTime += deltaTime;
			scene.ForEachComponent<comp::Light>([&](Entity e, comp::Light& l)
				{
					switch (l.lightData.type)
					{
					case TypeLight::DIRECTIONAL:
					{
						l.lightData.direction = { -1.0f, 0.0f, 0.f, 0.f };
						sm::Vector3 dir = sm::Vector3::TransformNormal(sm::Vector3(l.lightData.direction), sm::Matrix::CreateRotationZ(dx::XMConvertToRadians(ROTATION) * (m_elapsedCycleTime)));

						l.lightData.direction = sm::Vector4(dir.x, dir.y, dir.z, 0.0f);
						sm::Vector3 pos = l.lightData.position;
						pos = playerPos - dir * 200;
						l.lightData.position = sm::Vector4(pos);
						l.lightData.position.w = 1.f;
						break;
					}
					case TypeLight::POINT:
					{
						l.lightData.enabled = false;
						break;
					}
					default:
						break;
					}
					e.GetComponent<comp::SphereCollider>()->Center = sm::Vector3(l.lightData.position);
				});
		}
	}
}

void Game::OnShutdown()
{
	m_players.clear();
	m_mapEntity.Destroy();
	m_models.clear();
	m_LOSColliders.clear();
}

void Game::CheckIncoming(message<GameMsg>& msg)
{
	switch (msg.header.id)
	{
	case GameMsg::Client_Accepted:
	{
		LOG_INFO("You are validated!");
		break;
	}
	case GameMsg::Server_GetPing:
	{
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
		m_client.m_latency = int(std::chrono::duration<double>(timeNow - this->m_timeThen).count() * 1000);
		break;
	}
	case GameMsg::Server_AssignID:
	{
		msg >> this->m_localPID;

		LOG_INFO("YOUR ID IS: %lu", this->m_localPID);
		break;
	}
	case GameMsg::Game_Snapshot:
	{
		m_savedInputs.clear();
		uint32_t count;
		msg >> count;

		for (uint32_t i = 0; i < count; i++)
		{
			uint32_t entityID;
			msg >> entityID;

			Entity entity;
			if (m_gameEntities.find(entityID) != m_gameEntities.end())
			{
				entity = m_gameEntities.at(entityID);
				UpdateEntityFromMessage(entity, msg);
			}
			else
			{
				LOG_WARNING("Updating: Entity %u not in m_gameEntities, should not happen...", entityID);
			}
		}

		break;
	}
	case GameMsg::Game_UpdateComponent:
	{
		uint32_t count; // Could be more than one Entity
		msg >> count;

		for (uint32_t i = 0; i < count; i++)
		{
			uint32_t entityID;
			msg >> entityID;

			Entity entity;
			if (m_gameEntities.find(entityID) != m_gameEntities.end())
			{
				entity = m_gameEntities.at(entityID);
				UpdateEntityFromMessage(entity, msg);
			}
			else
			{
				LOG_WARNING("Updating component: Entity %u not in m_gameEntities, should not happen...", entityID);
			}
		}

		break;
	}
	case GameMsg::Game_AddEntity:
	{
		uint32_t count; // Could be more than one Entity
		msg >> count;
#ifdef _DEBUG
		LOG_INFO("Received %u entities", count);
#endif

		for (uint32_t i = 0; i < count; i++)
		{
			Entity e = GetScene("Game").CreateEntity();
			uint32_t entityID;
			msg >> entityID;
			e.AddComponent<comp::Network>(entityID);
			UpdateEntityFromMessage(e, msg);

			m_gameEntities.insert(std::make_pair(entityID, e));
			if (e.GetComponent<comp::Network>()->id == m_localPID)
			{
#ifdef _DEBUG
				LOG_INFO("You added yourself, congratulations!");
#endif
				m_players[m_localPID] = e;

				//if (e.GetComponent<comp::Transform>())
				//{
				//	e.AddComponent <comp::EmitterParticle>("thisisfine.png", "thisisfine_Opacity.png", 100, PARTICLEMODE::SMOKE);
				//}
			}
			else if (e.GetComponent<comp::Player>())
			{
				LOG_INFO("A remote player added!");
				m_players[e.GetComponent<comp::Network>()->id] = e;
			}
		}

		break;
	}
	case GameMsg::Game_RemoveEntity:
	{
		uint32_t count;
		msg >> count;
		for (uint32_t i = 0; i < count; i++)
		{
			uint32_t id;
			msg >> id;
			if (m_gameEntities.find(id) != m_gameEntities.end())
			{
				// Spawn blood splat when enemy dies.
				if (m_gameEntities.at(id).GetComponent<comp::Transform>() && m_gameEntities.at(id).GetComponent<comp::Health>())
				{
					Entity bloodDecal = GetCurrentScene()->CreateEntity();
					bloodDecal.AddComponent<comp::Decal>(*m_gameEntities.at(id).GetComponent<comp::Transform>());
				}

				m_gameEntities.at(id).Destroy();
				m_gameEntities.erase(id);
			}
			// Was the entity a player?
			if (m_players.find(id) != m_players.end())
			{
				m_players.at(id).Destroy();
				m_players.erase(id);
			}
		}
#ifdef _DEBUG
		LOG_INFO("Removed %u entities", count);
#endif
		break;
	}
	case GameMsg::Game_BackToLobby:
	{
		auto it = m_players.begin();
		while (it != m_players.end())
		{
			it->second.GetComponent<comp::Player>()->isReady = false;
			it++;
		}
		SetScene("Lobby");
		break;
	}
	case GameMsg::Game_Over:
	{
		SetScene("GameOver");
		break;
	}
	case GameMsg::Lobby_Accepted:
	{
		msg >> m_gameID;

		this->SetScene("Loading");
		sceneHelp::LoadGameScene(this);
		sceneHelp::LoadResources(this);
		sceneHelp::LoadMapColliders(this);

#ifdef _DEBUG
		LOG_INFO("Successfully loaded all Assets!");
#endif
		SetScene("Lobby");

		LOG_INFO("You are now in lobby: %lu", m_gameID);
		break;
	}
	case GameMsg::Lobby_Invalid:
	{
		std::string err;
		msg >> err;
		SetScene("JoinLobby");
		LOG_WARNING("%s", err.c_str());
		break;
	}
	case GameMsg::Lobby_AcceptedLeave:
	{
		LOG_WARNING("Left Lobby %u", m_gameID);
		m_gameID = -1;
		SetScene("JoinLobby");
		rtd::TextField* textField = dynamic_cast<rtd::TextField*>(GetScene("JoinLobby").GetCollection("LobbyFields")->elements[0].get());
		if (textField)
		{
			textField->SetPresetText("");
		}
		break;
	}
	case GameMsg::Game_Start:
	{
		GetScene("Game").ForEachComponent<comp::Tag<TagType::CAMERA>>([&](Entity entt, comp::Tag<TagType::CAMERA>& t)
			{
				comp::Camera3D* c = entt.GetComponent<comp::Camera3D>();
				if (c)
				{
					c->camera.SetFollowEntity(m_players.at(m_localPID));
				}
			});

		SetScene("Game");
		thread::RenderThreadHandler::Get().GetRenderer()->GetDoFPass()->SetDoFType(DoFType::ADAPTIVE);
		break;
	}
	case GameMsg::Game_WaveTimer:
	{
		msg >> m_serverCycle;
		msg >> m_waveTimer;
		switch (m_serverCycle)
		{
		case Cycle::DAY:
		{
			Scene& scene = GetScene("Game");
			// Change light when day.
			scene.ForEachComponent<comp::Light>([&](Entity e, comp::Light& l)
				{
					if (l.lightData.type == TypeLight::DIRECTIONAL)
					{
						l.lightData.enabled = true;
					}
				});
			break;
		}
		case Cycle::NIGHT:
		{
			Scene& scene = GetScene("Game");
			// Change light on Night.
			m_elapsedCycleTime = 0.0f;
			scene.ForEachComponent<comp::Light>([&](Entity e, comp::Light& l)
				{
					switch (l.lightData.type)
					{
					case TypeLight::DIRECTIONAL:
					{
						l.lightData.enabled = false;
						break;
					}
					case TypeLight::POINT:
					{
						l.lightData.enabled = true;
						break;
					}
					}
				});
			break;
		}
		case Cycle::MORNING:
		{
			Scene& scene = GetScene("Game");
			// Change light on Morning.
			scene.ForEachComponent<comp::Light>([&](Entity e, comp::Light& l)
				{
					if (l.lightData.type == TypeLight::DIRECTIONAL)
					{
						l.lightData.enabled = true;
					}
				});
			break;
		}
		default:
			break;
		}
		break;
	}
	case GameMsg::Lobby_Update:
	{
		uint8_t count;
		msg >> count;

		for (uint8_t i = count; i > 0; i--)
		{
			char nameTemp[12] = {};
			uint32_t playerID;
			comp::Player::Class classType;
			bool isReady = false;
			msg >> isReady;
			msg >> classType;
			msg >> nameTemp;
			msg >> playerID;
			std::string name(nameTemp);

			if (m_players.find(playerID) != m_players.end())
			{
				dynamic_cast<rtd::Text*>(GetScene("Lobby").GetCollection("playerIcon" + std::to_string(i))->elements[1].get())->SetText(name);
				rtd::Text* plT = dynamic_cast<rtd::Text*>(GetScene("Game").GetCollection("dynamicPlayer" + std::to_string(i) + "namePlate")->elements[0].get());
				rtd::Picture* plP = dynamic_cast<rtd::Picture*>(GetScene("Lobby").GetCollection("playerIcon" + std::to_string(i))->elements[2].get());
				if (plT)
				{
					plT->SetText(name);
					plT->SetStretch(D2D1Core::GetDefaultFontSize() * plT->GetText().length(), D2D1Core::GetDefaultFontSize());
				}

				if (plP)
				{
					switch (classType)
					{
					case comp::Player::Class::MAGE:
					{
						plP->SetTexture("WizardIcon.png");
						break;
					}
					case comp::Player::Class::WARRIOR:
					{
						plP->SetTexture("WarriorIcon.png");
						break;
					}
					}
				}

				if (isReady)
				{
					dynamic_cast<rtd::Picture*>(GetScene("Lobby").GetCollection("playerIcon" + std::to_string(i))->elements[3].get())->SetVisiblity(true);
				}
				else
					dynamic_cast<rtd::Picture*>(GetScene("Lobby").GetCollection("playerIcon" + std::to_string(i))->elements[3].get())->SetVisiblity(false);
			}
		}

		if (m_players.find(m_localPID) != m_players.end())
		{
			comp::Player* player = m_players.at(m_localPID).GetComponent<comp::Player>();
			rtd::Button* readyText = dynamic_cast<rtd::Button*>(GetScene("Lobby").GetCollection("StartGame")->elements[0].get());
			if (readyText)
			{
				if (player->isReady)
				{
					readyText->GetPicture()->SetTexture("NotReady.png");
				}
				else
				{
					readyText->GetPicture()->SetTexture("Ready.png");
				}
			}
		}

		dynamic_cast<rtd::Text*>(GetScene("Lobby").GetCollection("LobbyDesc")->elements[1].get())->SetText("Lobby ID: " + std::to_string(m_gameID));

		for (uint32_t i = 0; i < count; i++)
		{
			Collection2D* coll = GetScene("Lobby").GetCollection("playerIcon" + std::to_string(i + 1));
			coll->elements[0].get()->SetVisiblity(true);
			coll->elements[1].get()->SetVisiblity(true);
			coll->elements[2].get()->SetVisiblity(true);
		}
		for (uint32_t i = count; i < MAX_PLAYERS_PER_LOBBY; i++)
		{
			GetScene("Lobby").GetCollection("playerIcon" + std::to_string(i + 1))->Hide();
		}

		GameSystems::UpdateHealthbar(this);
		break;

	}
	case GameMsg::Game_Money:
	{
		msg >> m_money;
		rtd::MoneyUI* elem = dynamic_cast<rtd::MoneyUI*>(GetScene("Game").GetCollection("MoneyUI")->elements[0].get());
		if (elem)
		{
			elem->SetNewMoney(m_money);
		}
		break;
	}
	case GameMsg::Game_ChangeAnimation:
	{
		uint32_t id;
		EAnimationType animtype = EAnimationType::NONE;
		msg >> id >> animtype;

		if (m_gameEntities.find(id) != m_gameEntities.end())
		{
			comp::Animator* animComp = m_gameEntities.at(id).GetComponent<comp::Animator>();
			if (animComp)
			{
				std::shared_ptr<RAnimator> anim = animComp->animator;
				if (anim)
				{
					anim->ChangeAnimation(animtype);
				}
			}
		}
		break;
	}
	case GameMsg::Game_Cooldown:
	{
		uint32_t count = 0;
		msg >> count;

		for (uint32_t i = 0; i < count; i++)
		{
			float cooldown = 0.0f;
			msg >> cooldown;

			AbilityIndex index = AbilityIndex::DEFAULT;
			msg >> index;
			switch (index)
			{
			case AbilityIndex::Primary:
			{
				m_primaryCooldown = cooldown;
				break;
			}
			case AbilityIndex::Secondary:
			{
				m_secondaryCooldown = cooldown;
				break;
			}
			case AbilityIndex::Dodge:
			{
				m_dodgeCooldown = cooldown;
				break;
			}
			default:
				break;
			}
		}
		break;
	}
	case GameMsg::Game_StartSpectate:
	{
		m_players.at(m_localPID).GetComponent<comp::Player>()->state = comp::Player::State::SPECTATING;
		auto it = m_players.begin();
		while (it != m_players.end())
		{
			if (it->first != m_localPID && it->second.GetComponent<comp::Health>()->isAlive)
			{
				GetScene("Game").ForEachComponent<comp::Tag<TagType::CAMERA>>([&](Entity entt, comp::Tag<TagType::CAMERA>& t)
					{
						comp::Camera3D* c = entt.GetComponent<comp::Camera3D>();
						if (c)
						{
							c->camera.SetFollowEntity(m_players.at(it->first));
						}
					});
				m_spectatingID = it->first;
				break;
			}
			it++;
		}
		break;
	}
	case GameMsg::Game_StopSpectate:
	{
		m_players.at(m_localPID).GetComponent<comp::Player>()->state = comp::Player::State::IDLE;
		GetScene("Game").ForEachComponent<comp::Tag<TagType::CAMERA>>([&](Entity entt, comp::Tag<TagType::CAMERA>& t)
			{
				comp::Camera3D* c = entt.GetComponent<comp::Camera3D>();
				if (c)
				{
					c->camera.SetFollowEntity(m_players.at(m_localPID));
				}
			});
		m_spectatingID = -1;
		break;
	}
	}
}
void Game::PingServer()
{
	message<GameMsg> msg = {};
	msg.header.id = GameMsg::Server_GetPing;
	msg << this->m_localPID;

	LOG_INFO("Pinging server!");

	this->m_timeThen = std::chrono::system_clock::now();
	m_client.Send(msg);
}

void Game::JoinLobby(uint32_t lobbyID)
{
	if (m_gameID == (uint32_t)-1)
	{
		message<GameMsg> msg;
		msg.header.id = GameMsg::Lobby_Join;
		msg << m_playerName;
		msg << this->m_localPID << lobbyID;
		m_client.Send(msg);
	}
	else
	{
		LOG_WARNING("Request denied: You are already in a lobby");
	}
}

void Game::CreateLobby()
{
	if (m_gameID == (uint32_t)-1)
	{
		message<GameMsg> msg;

		msg.header.id = GameMsg::Lobby_Create;
		msg << m_playerName;
		msg << this->m_localPID;
		m_client.Send(msg);
	}
	else
	{
		LOG_WARNING("Request denied: You are already in a lobby");
	}
}

const Mode& Game::GetCurrentMode() const
{
	return m_mode;
}

const Cycle& Game::GetCurrentCycle() const
{
	return m_serverCycle;
}

void Game::SetMode(const Mode& mode)
{
	m_mode = mode;
}

const uint32_t& Game::GetMoney() const
{
	return m_money;
}

void Game::OnClientDisconnect()
{
	this->m_gameID = -1;
	this->m_localPID = -1;

	// remove all network entities
	GetScene("Game").ForEachComponent<comp::Network>([](Entity& e, comp::Network& net)
		{
			e.Destroy();
		}
	);

	rtd::TextField* textField = dynamic_cast<rtd::TextField*>(GetScene("JoinLobby").GetCollection("nameInput")->elements[0].get());
	if (textField)
	{
		textField->SetPresetText("");
	}
	textField = dynamic_cast<rtd::TextField*>(GetScene("JoinLobby").GetCollection("LobbyFields")->elements[0].get());
	if (textField)
	{
		textField->SetPresetText("");
	}

	rtd::TextField* ipInput = dynamic_cast<rtd::TextField*>(GetScene("MainMenu").GetCollection("ConnectFields")->elements[0].get());
	ipInput->SetActive();

	SetScene("MainMenu");

	m_client.m_qPrioMessagesIn.clear();
	m_client.m_qMessagesIn.clear();
	m_players.clear();
	m_gameEntities.clear();
	LOG_INFO("Disconnected from server!");
}

void Game::SendStartGame()
{
	network::message<GameMsg> msg;
	msg.header.id = GameMsg::Game_PlayerReady;
	msg << m_localPID << m_gameID;

	m_client.Send(msg);
}

void Game::SendSelectedClass(comp::Player::Class classType)
{
	network::message<GameMsg> msg;
	msg.header.id = GameMsg::Game_ClassSelected;
	msg << classType << m_localPID << m_gameID;

	m_client.Send(msg);
}

ParticleSystem* Game::GetParticleSystem()
{
	return &m_particles;
}

Entity& Game::GetLocalPlayer()
{
	return this->m_players.at(m_localPID);
}

void Game::UseShop(const ShopItem& whatToBuy)
{
	network::message<GameMsg> msg;
	msg.header.id = GameMsg::Game_UseShop;
	msg << whatToBuy << m_localPID << m_gameID;

	m_client.Send(msg);
}

void Game::UpgradeDefence(const uint32_t& id)
{
	network::message<GameMsg> msg;
	msg.header.id = GameMsg::Game_UpgradeDefence;
	msg << id << m_localPID << m_gameID;

	m_client.Send(msg);
}

void Game::UpdateEntityFromMessage(Entity e, message<GameMsg>& msg)
{
	uint32_t bits;
	msg >> bits;
	std::bitset<ecs::Component::COMPONENT_MAX> compSet(bits);

	for (int i = ecs::Component::COMPONENT_COUNT - 1; i >= 0; i--)
	{
		if (compSet.test(i))
		{
			switch (i)
			{
			case ecs::Component::TRANSFORM:
			{
				comp::Transform t;
				msg >> t;
				t.rotation.Normalize();
				e.AddComponent<comp::Transform>(t);
				break;
			}
			case ecs::Component::VELOCITY:
			{
				comp::Velocity v;
				msg >> v;
				e.AddComponent<comp::Velocity>(v);
				break;
			}
			case ecs::Component::MESH_NAME:
			{
				NameType name;
				msg >> name;
				std::string nameString;
				switch (name)
				{
				case NameType::MESH_DEFENCE:
				{
					nameString = "Defence.obj";
					break;
				}
				case NameType::MESH_KNIGHT:
				{
					nameString = "Knight.fbx";
					break;
				}
				case NameType::MESH_MONSTER:
				{
					nameString = "Monster.fbx";
					break;
				}
				case NameType::MESH_SPHERE:
				{
					nameString = "Sphere.obj";
					break;
				}
				default:
				{
					nameString = "Cube.obj";
					break;
				}
				}

				std::shared_ptr<RModel> model = ResourceManager::Get().CopyResource<RModel>(nameString, true);
				if (model)
				{
					e.AddComponent<comp::Renderable>()->model = model;
				}
				break;
			}
			case ecs::Component::ANIMATOR_NAME:
			{
				AnimName name;
				msg >> name;
				std::string nameString;
				switch (name)
				{
				case AnimName::ANIM_KNIGHT:
				{
					nameString = "Knight.anim";
					break;
				}
				case AnimName::ANIM_MONSTER:
				{
					nameString = "Monster.anim";
					break;
				}
				}
				if (nameString.length() > 0)
				{
					std::shared_ptr<RAnimator> animator = ResourceManager::Get().CopyResource<RAnimator>(nameString, true);
					if (animator)
					{
						animator->RandomizeTime();
						e.AddComponent<comp::Animator>()->animator = animator;
					}
				}
				break;
			}
			case ecs::Component::HEALTH:
			{
				comp::Health heal;
				msg >> heal;
				e.AddComponent<comp::Health>(heal);
				break;
			}
			case ecs::Component::BOUNDING_ORIENTED_BOX:
			{
				dx::BoundingOrientedBox box;
				msg >> box;
				comp::OrientedBoxCollider* collider = e.AddComponent<comp::OrientedBoxCollider>();
				collider->Center = box.Center;
				collider->Extents = box.Extents;
				collider->Orientation = box.Orientation;
				break;
			}
			case ecs::Component::BOUNDING_SPHERE:
			{
				dx::BoundingSphere s;
				msg >> s;
				comp::SphereCollider* collider = e.AddComponent<comp::SphereCollider>();
				collider->Center = s.Center;
				collider->Radius = s.Radius;
				break;
			}
			case ecs::Component::PARTICLEMITTER:
			{
				comp::PARTICLEEMITTER p;
				msg >> p;
				e.AddComponent<comp::EmitterParticle>(p.positionOffset, (int)p.nrOfParticles, p.sizeMulitplier, p.type, p.lifeTime, p.speed, p.hasDeathTimer);
				break;
			}
			case ecs::Component::PLAYER:
			{
				comp::Player p;
				msg >> p;
				e.AddComponent<comp::Player>(p);

				break;
			}
			case ecs::Component::COST:
			{
				comp::Cost c;
				msg >> c;
				e.AddComponent<comp::Cost>(c);
				break;
			}
			default:
				LOG_WARNING("Retrieved unimplemented component %u", i);
				break;
			}
		}
	}
}

void Game::UpdateInput()
{
	m_inputState.axisHorizontal = InputSystem::Get().GetAxis(Axis::HORIZONTAL);
	m_inputState.axisVertical = InputSystem::Get().GetAxis(Axis::VERTICAL);
	if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::HELD))
	{
		m_inputState.leftMouse = true;
	}
	if (InputSystem::Get().CheckMouseKey(MouseKey::RIGHT, KeyState::PRESSED))
	{
		m_inputState.rightMouse = true;
		if (m_localPID != -1 && m_players.size() > 0 && m_players.at(m_localPID).GetComponent<comp::Player>()->state == comp::Player::State::SPECTATING)
		{

			auto it = m_players.begin();
			while (it != m_players.end())
			{
				if (it->first != m_localPID && it->first != m_spectatingID && it->second.GetComponent<comp::Health>()->isAlive)
				{
					GetScene("Game").ForEachComponent<comp::Tag<TagType::CAMERA>>([&](Entity entt, comp::Tag<TagType::CAMERA>& t)
						{
							comp::Camera3D* c = entt.GetComponent<comp::Camera3D>();
							if (c)
							{
								c->camera.SetFollowEntity(m_players.at(it->first));
							}
						});
					m_spectatingID = it->first;
					break;
				}
				it++;
			}

		}
	}
	m_inputState.mouseRay = InputSystem::Get().GetMouseRay();

	if (m_mode == Mode::DESTROY_MODE)
	{
		if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::R, KeyState::PRESSED))
		{
			m_inputState.key_r = true;
		}
	}
	else if (m_mode == Mode::BUILD_MODE)
	{
		if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::B, KeyState::PRESSED))
		{
			m_inputState.key_b = true;
		}
	}
	if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::LeftShift, KeyState::PRESSED))
	{
		m_inputState.key_shift = true;
	}

	m_savedInputs.push_back(m_inputState);


	//TEMP PLZ REMOVE AFTER WE COME TO AN AGREEMENT ON WHICH DOF EFFECT TO USE
	if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::D1, KeyState::PRESSED))
	{
		thread::RenderThreadHandler::Get().GetRenderer()->GetDoFPass()->SetDoFType(DoFType::ADAPTIVE);
	}

	if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::D2, KeyState::PRESSED))
	{
		thread::RenderThreadHandler::Get().GetRenderer()->GetDoFPass()->SetDoFType(DoFType::VIGNETTE);
	}
}