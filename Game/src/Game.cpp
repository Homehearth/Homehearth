#include "Game.h"
#include "GameSystems.h"
#include "Button.h"
#include "TextField.h"
#include "SceneHelper.h"
#include "Healthbar.h"
#include "MoneyUI.h"
#include "OptionSystem.h"
#include "AbilityUI.h"
#include "LobbyUI.h"

using namespace std::placeholders;

Game::Game()
	: m_client(std::bind(&Game::CheckIncoming, this, _1), std::bind(&Game::OnClientDisconnect, this))
	, Engine()
{
	this->m_localPID = -1;
	this->m_spectatingID = -1;
	this->m_money = 0;
	this->m_waveCounter = 0;
	this->m_gameID = -1;
	this->m_inputState = {};
	this->m_isSpectating = false;
}

uint32_t& Game::GetWaveCounter()
{
	return m_waveCounter;
}

Game::~Game()
{
	if (m_client.IsConnected())
	{
		m_client.Disconnect();
	}

	OptionSystem::Get().SetOption("MasterVolume", std::to_string(SoundHandler::Get().GetMasterVolume()));
	OptionSystem::Get().OnShutdown();
}

void Game::UpdateNetwork(float deltaTime)
{
	static float pingCheck = 0.f;
	static float refreshLobbyList = 0.f;
	const float TARGET_PING_TIME = 5.0f;
	const float TARGET_REFRESH = 0.2f;
	if (m_client.IsConnected())
	{
		m_client.Update();

		//pingCheck += deltaTime;

		//if (pingCheck > TARGET_PING_TIME)
		//{
		//	//this->PingServer();
		//	pingCheck -= TARGET_PING_TIME;
		//}

		refreshLobbyList += deltaTime;

		if (refreshLobbyList >= TARGET_REFRESH)
		{
			refreshLobbyList -= TARGET_REFRESH;

			if (GetCurrentScene() == &(GetScene("JoinLobby")))
			{
				message<GameMsg> msg;
				msg.header.id = GameMsg::Lobby_RefreshList;
				msg << m_localPID;
				m_client.Send(msg);
			}
		}

		if (GetCurrentScene() == &GetScene("Game"))
		{
			if (GetCurrentScene()->GetCurrentCamera()->GetCameraType() == CAMERATYPE::PLAY && !m_isSpectating)
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
	// Scene logic
	sceneHelp::CreateLobbyScene(this);
	sceneHelp::CreateGameScene(this);
	sceneHelp::CreateMainMenuScene(this);
	sceneHelp::CreateJoinLobbyScene(this);
	sceneHelp::CreateGameOverScene(this);

	sceneHelp::CreateOptionsScene(this);

	// Set Current Scene
	SetScene("MainMenu");

	return true;
}

void Game::OnUserUpdate(float deltaTime)
{
	this->UpdateInput();

	//scene.ForEachComponent<comp::Light>([&](Entity e, comp::Light& l)
	//	{
	//		e.GetComponent<comp::SphereCollider>()->Center = sm::Vector3(l.lightData.position);
	//	});

	Scene& scene = GetScene("Game");
	Entity e;
	if (GetLocalPlayer(e))
	{
		comp::KillDeaths* p = e.GetComponent<comp::KillDeaths>();

		// Update the death and kill counter.
		if (p)
		{
			Collection2D* killColl = scene.GetCollection("ZKillCounter");
			Collection2D* deathColl = scene.GetCollection("ZDeathCounter");
			if (killColl && deathColl)
			{
				dynamic_cast<rtd::Text*>(killColl->elements[1].get())->SetText(std::to_string(p->kills));
				dynamic_cast<rtd::Text*>(deathColl->elements[1].get())->SetText(std::to_string(p->deaths));
			}
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
		rtd::Text* mainMenuErrorText = dynamic_cast<rtd::Text*>(GetScene("MainMenu").GetCollection("ConnectFields")->elements[6].get());
		mainMenuErrorText->SetVisiblity(false);
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
		uint32_t count;
		msg >> count;

		for (uint32_t i = 0; i < count; i++)
		{
			uint32_t entityID;
			msg >> entityID;

			Entity entity;
			bool skip = false;
			if (m_gameEntities.find(entityID) != m_gameEntities.end())
			{
				entity = m_gameEntities.at(entityID);
			}
			else
			{
				LOG_WARNING("Updating: Entity %u not in m_gameEntities, skipping over this entity!", entityID);
				skip = true;
			}

			UpdateEntityFromMessage(entity, msg, skip);
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
			bool skip = false;
			if (m_gameEntities.find(entityID) != m_gameEntities.end())
			{
				entity = m_gameEntities.at(entityID);
			}
			else
			{
				skip = true;
				LOG_WARNING("Updating component: Entity %u not in m_gameEntities, skipping over this comp!", entityID);
			}
			UpdateEntityFromMessage(entity, msg, skip);
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
		int removed = 0;
		for (uint32_t i = 0; i < count; i++)
		{
			uint32_t id;
			msg >> id;
			if (m_gameEntities.find(id) != m_gameEntities.end())
			{
				if (m_players.find(id) != m_players.end())
				{
					m_players.erase(id);
				}

				// Spawn blood splat when enemy dies.
				if (m_gameEntities.at(id).GetComponent<comp::Tag<BAD>>())
				{
					Entity bloodDecal = GetCurrentScene()->CreateEntity();
					bloodDecal.AddComponent<comp::Decal>(*m_gameEntities.at(id).GetComponent<comp::Transform>());
				}

				comp::Player* p = m_gameEntities.at(id).GetComponent<comp::Player>();
				if (p)
				{
					GetScene("Game").GetCollection("Aplayer" + std::to_string(static_cast<uint16_t>(p->playerType)) + "Info")->Hide();
					GetScene("Game").GetCollection("AdynamicPlayer" + std::to_string(static_cast<uint16_t>(p->playerType)) + "namePlate")->Hide();
				}
				m_gameEntities.at(id).Destroy();
				m_gameEntities.erase(id);
				removed++;
			}
		}
#ifdef _DEBUG
		LOG_INFO("Removed %d entities", removed);
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
	case GameMsg::Game_PlaySound:
	{
		uint32_t nrOfSounds = 0u;
		msg >> nrOfSounds;

		audio_t data = {};

		const auto SH = &SoundHandler::Get();

		for (uint32_t i = 0; i < nrOfSounds; i++)
		{
			// Extract Sound Information.
			msg >> data.playLooped;
			msg >> data.shouldBroadcast;
			msg >> data.isUnique;
			msg >> data.is3D;
			msg >> data.minDistance;
			msg >> data.volume;
			msg >> data.position;
			msg >> data.type;

			switch (data.type)
			{
				//--------------------	PLAYER	--------------------------------------
			case ESoundEvent::Player_OnMeleeAttack:
				SH->PlaySound("Player_OnMeleeAttack", data);
				break;
			case ESoundEvent::Player_OnMeleeAttackHit:
			{
				int version = rand() % 3 + 1;
				std::string onAttackName = "Player_OnMeleeAttackHit" + std::to_string(version);
				SH->PlaySound(onAttackName, data);
			}
			break;
			case ESoundEvent::Player_OnMovement:
				SH->PlaySound("Player_OnMovement", data);
				break;
			case ESoundEvent::Player_OnRangeAttack:
				SH->PlaySound("Player_OnRangeAttack", data);
				break;
			case ESoundEvent::Player_OnRangeAttackHit:
				SH->PlaySound("Player_OnRangeAttackHit", data);
				break;
			case ESoundEvent::Player_OnDmgDealt:
				SH->PlaySound("Player_OnDmgDealt", data);
				break;
			case ESoundEvent::Player_OnDmgRecieved:
				SH->PlaySound("Player_OnDmgRecieved", data);
				break;
			case ESoundEvent::Player_OnHealing:
				SH->PlaySound("Player_OnHealing", data);
				break;
			case ESoundEvent::Player_OnCastDash:
			{
				int version = rand() % 10;
				if (version == 0)
					SH->PlaySound("Player_OnCastDash1", data);
				else
					SH->PlaySound("Player_OnCastDash", data);
			}
			break;
			case ESoundEvent::Player_OnCastBlink:
				SH->PlaySound("Player_OnCastBlink", data);
				break;
			case ESoundEvent::Player_OnDeath:
				SH->PlaySound("Player_OnDeath", data);
				break;
			case ESoundEvent::Player_OnRespawn:
				SH->PlaySound("Player_OnRespawn", data);
				break;
				//--------------------	ENEMY	--------------------------------------
			case ESoundEvent::Enemy_OnMovement:
				SH->PlaySound("Enemy_OnMovement", data);
				break;
			case ESoundEvent::Enemy_OnMeleeAttack:
			{
				int version = rand() % 7 + 1;
				std::string onAttackName = "Enemy_OnMeleeAttack" + std::to_string(version);
				SH->PlaySound(onAttackName, data);
			}
			break;
			case ESoundEvent::Enemy_OnRangeAttack:
				SH->PlaySound("Enemy_RangeAttack", data);
				break;
			case ESoundEvent::Enemy_OnDmgDealt:
				SH->PlaySound("Enemy_OnDmgDealt", data);
				break;
			case ESoundEvent::Enemy_OnDmgRecieved:
				SH->PlaySound("Enemy_OnDmgRecieved", data);
				break;
			case ESoundEvent::Enemy_OnDeath:
				SH->PlaySound("Enemy_OnDeath", data);
				break;
				//--------------------	GAME	--------------------------------------
			case ESoundEvent::Game_OnDefencePlaced:
				SH->PlaySound("Game_OnDefencePlaced", data);
				break;
			case ESoundEvent::Game_OnDefenceDestroyed:
				SH->PlaySound("Game_OnDefenceDestroyed", data);
				break;
			case ESoundEvent::Game_OnHouseDestroyed:
				SH->PlaySound("Game_OnHouseDestroyed", data);
				break;
			case ESoundEvent::Game_OnPurchase:
				SH->PlaySound("Game_OnPurchase", data);
				break;
			default:
				break;
			}
		}

		break;
	}
	case GameMsg::Game_Over:
	{
		uint32_t gatheredMoney, wavesSurvived;
		msg >> wavesSurvived >> gatheredMoney;

		SoundHandler::Get().GetCurrentMusic()->stop();

		audio_t audio = {};
		audio.isUnique = true;
		audio.playLooped = false;
		SoundHandler::Get().PlaySound("Player_OnDeath", audio);
		audio.volume = 0.5f;
		SoundHandler::Get().PlaySound("OnGameOver", audio);
		rtd::Text* mainMenuErrorText = dynamic_cast<rtd::Text*>(GetScene("MainMenu").GetCollection("ConnectFields")->elements[6].get());
		mainMenuErrorText->SetVisiblity(false);
		Scene& gameScene = GetScene("Game");
		gameScene.GetCollection("SpectateUI")->elements[0]->SetVisiblity(false);
		gameScene.GetCollection("SpectateUI")->elements[1]->SetVisiblity(false);
		rtd::Text* scoreText = dynamic_cast<rtd::Text*>(GetScene("GameOver").GetCollection("GameOver")->elements[1].get());
		scoreText->SetText("Score: " + std::to_string(gatheredMoney));
		rtd::Text* wavesText = dynamic_cast<rtd::Text*>(GetScene("GameOver").GetCollection("GameOver")->elements[2].get());
		wavesText->SetText("Waves: " + std::to_string(wavesSurvived));
		SetScene("GameOver");

		m_gameID = -1;

		auto it = m_gameEntities.begin();

		while (it != m_gameEntities.end())
		{
			it->second.Destroy();
			it = m_gameEntities.erase(it);
		}

		m_cycler.SetTime(MORNING);

		it = m_players.begin();
		int i = 1;
		Scene& lobbyScene = GetScene("Lobby");
		while (it != m_players.end())
		{
			it->second.Destroy();
			it = m_players.erase(it);

			GetScene("Game").GetCollection("Aplayer" + std::to_string(i + 1) + +"Info")->Hide();
			GetScene("Game").GetCollection("AdynamicPlayer" + std::to_string(i + 1) + "namePlate");
			lobbyScene.GetCollection("playerIcon" + std::to_string(i))->Hide();
		}

		m_isSpectating = false;

		rtd::Button* readyText = dynamic_cast<rtd::Button*>(lobbyScene.GetCollection("StartGame")->elements[0].get());
		readyText->GetPicture()->SetTexture("Ready.png");

		rtd::Button* wizardButton = dynamic_cast<rtd::Button*>(lobbyScene.GetCollection("ClassButtons")->elements[0].get());
		wizardButton->GetBorder()->SetVisiblity(false);
		rtd::Button* warriorButton = dynamic_cast<rtd::Button*>(lobbyScene.GetCollection("ClassButtons")->elements[1].get());
		warriorButton->GetBorder()->SetVisiblity(true);
		rtd::Picture* desc = dynamic_cast<rtd::Picture*>(lobbyScene.GetCollection("ClassTextCanvas")->elements[0].get());
		desc->SetTexture("WarriorDesc.png");

		break;
	}
	case GameMsg::Lobby_Accepted:
	{
		msg >> m_gameID;

		this->SetScene("Loading");
		if (!hasLoaded)
		{
			sceneHelp::LoadGameScene(this);
			sceneHelp::LoadResources(this);
			sceneHelp::LoadMapColliders(this);

			hasLoaded = true;
		}

#ifdef _DEBUG
		LOG_INFO("Successfully loaded all Assets!");
#endif
		SetScene("Lobby");
		LOG_INFO("You are now in lobby: %lu", m_gameID);
		break;
	}
	case GameMsg::Lobby_Invalid:
	{
		//std::string err;
		//msg >> err;
		SetScene("JoinLobby");
		//LOG_WARNING("%s", err.c_str());
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


		GetScene("Game").ForEachComponent<comp::Light>([](comp::Light& l)
			{
				if (l.lightData.type == TypeLight::POINT)
				{
					l.lightData.enabled = 0;
				}
			});
		Scene& scene = GetScene("Game");
		Collection2D* skipButtonUI = scene.GetCollection("SkipUI");
		rtd::Button* skipButton = dynamic_cast<rtd::Button*>(skipButtonUI->elements[0].get());
		rtd::Text* skipText = dynamic_cast<rtd::Text*>(skipButtonUI->elements[1].get());
		skipText->SetVisiblity(true);
		skipButton->SetVisiblity(true);

		SoundHandler::Get().SetCurrentMusic("MenuTheme");
		SetScene("Game");
		//thread::RenderThreadHandler::Get().GetRenderer()->GetDoFPass()->SetDoFType(DoFType::ADAPTIVE);
		Entity e;
		if (GetLocalPlayer(e))
		{
			comp::Player* p = e.GetComponent<comp::Player>();

			if (p)
			{
				if (p->classType == comp::Player::Class::WARRIOR)
				{
					dynamic_cast<rtd::AbilityUI*>(GetScene("Game").GetCollection("ZAbilityUI")->elements[1].get())->SetTexture("Attack2.png");
					dynamic_cast<rtd::AbilityUI*>(GetScene("Game").GetCollection("ZAbilityUI")->elements[2].get())->SetTexture("Block.png");
				}
				else
				{
					dynamic_cast<rtd::AbilityUI*>(GetScene("Game").GetCollection("ZAbilityUI")->elements[1].get())->SetTexture("Attack.png");
					dynamic_cast<rtd::AbilityUI*>(GetScene("Game").GetCollection("ZAbilityUI")->elements[2].get())->SetTexture("Heal.png");
				}
			}
		}

		m_primaryCooldown = 0.0f;
		m_secondaryCooldown = 0.0f;
		m_dodgeCooldown = 0.0f;
		m_waveCounter = 0;

		dynamic_cast<rtd::Text*>(GetScene("Game").GetCollection("ZWaveCounter")->elements[1].get())->SetText("0");

		this->m_inputState = { };
		SetScene("Game");
		break;
	}
	case GameMsg::Game_Spree:
	{
		msg >> m_currentSpree;
		rtd::Text* txt = dynamic_cast<rtd::Text*>(GetScene("Game").GetCollection("SpreeText")->elements[0].get());
		if (txt)
		{
			txt->SetText("X" + std::to_string(m_currentSpree));
		}
		break;
	}
	case GameMsg::Game_Time:
	{
		CyclePeriod timePeriod;
		float speed;
		bool hasChangedPeriod;
		msg >> hasChangedPeriod;
		msg >> timePeriod;
		msg >> speed;
		m_cycler.SetTimePeriod(timePeriod, hasChangedPeriod);
		m_cycler.SetCycleSpeed(speed);
		break;
	}
	case GameMsg::Game_Time_Update:
	{
		float speed;
		float time;
		msg >> time;
		msg >> speed;
		m_cycler.SetTime(time);
		m_cycler.SetCycleSpeed(speed);
		break;
	}
	case GameMsg::Lobby_RefreshList:
	{
		uint8_t playerCount;
		uint8_t sceneStatus;

		for (int i = MAX_LOBBIES - 1; i >= 0; i--)
		{
			msg >> sceneStatus >> playerCount;

			rtd::LobbyUI* lobbyUI = dynamic_cast<rtd::LobbyUI*>(GetScene("JoinLobby").GetCollection("LobbySelect")->elements[i].get());
			lobbyUI->UpdateLobbyPlayerCount(playerCount);
			lobbyUI->SetLobbyStatus(sceneStatus);
		}

		break;
	}
	case GameMsg::Lobby_Update:
	{
		uint8_t count;
		msg >> count;

		comp::Player::PlayerType playerTypes[MAX_PLAYERS_PER_LOBBY] = { comp::Player::PlayerType::NONE };

		for (uint8_t i = 0; i < count; i++)
		{
			char nameTemp[13] = {};
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
				comp::Player* p = m_players.at(playerID).GetComponent<comp::Player>();
				if (!p)
					break;
				playerTypes[static_cast<int>(p->playerType) - 1] = p->playerType;
				dynamic_cast<rtd::Text*>(GetScene("Lobby").GetCollection("playerIcon" + std::to_string(static_cast<uint16_t>(p->playerType)))->elements[1].get())->SetText(name);
				rtd::Text* plT = dynamic_cast<rtd::Text*>(GetScene("Game").GetCollection("AdynamicPlayer" + std::to_string(static_cast<uint16_t>(p->playerType)) + "namePlate")->elements[0].get());
				rtd::Picture* plP = dynamic_cast<rtd::Picture*>(GetScene("Lobby").GetCollection("playerIcon" + std::to_string(static_cast<uint16_t>(p->playerType)))->elements[2].get());

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
					dynamic_cast<rtd::Picture*>(GetScene("Lobby").GetCollection("playerIcon" + std::to_string(static_cast<uint16_t>(p->playerType)))->elements[3].get())->SetVisiblity(true);
				}
				else
					dynamic_cast<rtd::Picture*>(GetScene("Lobby").GetCollection("playerIcon" + std::to_string(static_cast<uint16_t>(p->playerType)))->elements[3].get())->SetVisiblity(false);
			}
		}

		if (m_players.find(m_localPID) != m_players.end())
		{
			comp::Player* player = m_players.at(m_localPID).GetComponent<comp::Player>();
			if (player)
			{
				rtd::Button* readyText = dynamic_cast<rtd::Button*>(GetScene("Lobby").GetCollection("StartGame")->elements[0].get());
				if (readyText)
				{
					if (player->isReady && m_players.size() > 1)
					{
						readyText->GetPicture()->SetTexture("NotReady.png");
					}
					else
					{
						readyText->GetPicture()->SetTexture("Ready.png");
					}
				}
			}
		}

		dynamic_cast<rtd::Text*>(GetScene("Lobby").GetCollection("LobbyDesc")->elements[1].get())->SetText("Lobby " + std::to_string(m_gameID + 1));

		for (uint32_t i = 0; i < MAX_PLAYERS_PER_LOBBY; i++)
		{
			switch (playerTypes[i])
			{
			case comp::Player::PlayerType::NONE:
			{
				GetScene("Lobby").GetCollection("playerIcon" + std::to_string(i + 1))->Hide();

				break;
			}
			default:
			{
				Collection2D* coll = GetScene("Lobby").GetCollection("playerIcon" + std::to_string(static_cast<int>(playerTypes[i])));
				coll->elements[0].get()->SetVisiblity(true);
				coll->elements[1].get()->SetVisiblity(true);
				coll->elements[2].get()->SetVisiblity(true);

				break;
			}
			}
		}
		break;

	}
	case GameMsg::Game_Money:
	{
		msg >> m_money;
		rtd::MoneyUI* elem = dynamic_cast<rtd::MoneyUI*>(GetScene("Game").GetCollection("ZMoneyUI")->elements[0].get());
		if (elem)
		{
			elem->SetNewMoney(m_money);
		}
		break;
	}
	case GameMsg::Game_ChangeAnimation:
	{
		uint32_t id;
		EAnimationType animType = EAnimationType::NONE;
		uint16_t count;
		msg >> count;

		for (uint16_t i = 0; i < count; i++)
		{
			msg >> id >> animType;

			if (m_gameEntities.find(id) != m_gameEntities.end())
			{
				comp::Animator* animComp = m_gameEntities.at(id).GetComponent<comp::Animator>();
				if (animComp)
				{
					std::shared_ptr<RAnimator> anim = animComp->animator;
					if (anim)
					{
						anim->ChangeAnimation(animType);
					}
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
			float maxCooldown = 0.0f;
			msg >> maxCooldown;
			float cooldown = 0.0f;
			msg >> cooldown;

			AbilityIndex index = AbilityIndex::DEFAULT;
			msg >> index;
			switch (index)
			{
			case AbilityIndex::Primary:
			{
				m_primaryCooldown = cooldown;
				m_primaryMaxCooldown = maxCooldown;
				break;
			}
			case AbilityIndex::Secondary:
			{
				m_secondaryCooldown = cooldown;
				m_secondaryMaxCooldown = maxCooldown;
				break;
			}
			case AbilityIndex::Dodge:
			{
				m_dodgeCooldown = cooldown;
				m_dodgeMaxCooldown = maxCooldown;
				break;
			}
			default:
				break;
			}
		}
		break;
	}
	default:
	{
		LOG_ERROR("This message has an unknown header, should not happen..");
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

void Game::SetPlayerWantsToSkip(bool value)
{
	m_players.at(m_localPID).GetComponent<comp::Player>()->wantsToSkipDay = value;
	if (value)
	{
		message<GameMsg> msg;
		msg.header.id = GameMsg::Game_PlayerSkipDay;
		msg << this->m_localPID << m_gameID;
		m_client.Send(msg);
		LOG_INFO("Player wants to skip to Night")
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

Cycler& Game::GetCycler()
{
	return m_cycler;
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

	SetScene("MainMenu");

	m_client.m_qPrioMessagesIn.clear();
	m_client.m_qMessagesIn.clear();
	m_players.clear();
	m_gameEntities.clear();
	SoundHandler::Get().SetCurrentMusic("MenuTheme");
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

bool Game::GetLocalPlayer(Entity& player)
{
	if (m_players.find(m_localPID) == m_players.end())
	{
		return false;
	}

	player = m_players.at(m_localPID);
	return true;
}

void Game::SetShopItem(const ShopItem& whatToBuy)
{
	if (m_players.find(m_localPID) != m_players.end())
	{
		comp::Player* p = m_players.at(m_localPID).GetComponent<comp::Player>();
		if (p)
		{
			p->shopItem = whatToBuy;
		}
	}

	network::message<GameMsg> msg;
	msg.header.id = GameMsg::Game_UpdateShopItem;
	msg << whatToBuy << m_localPID << m_gameID;
	m_client.Send(msg);
}

ShopItem Game::GetShopItem() const
{
	ShopItem item = ShopItem::None;
	if (m_players.find(m_localPID) != m_players.end())
	{
		comp::Player* p = m_players.at(m_localPID).GetComponent<comp::Player>();
		if (p)
		{
			item = p->shopItem;
		}
	}
	return item;
}

void Game::UpgradeDefence(const uint32_t& id)
{
	network::message<GameMsg> msg;
	msg.header.id = GameMsg::Game_UpgradeDefence;
	msg << id << m_localPID << m_gameID;

	m_client.Send(msg);
}

void Game::UpdateEntityFromMessage(Entity e, message<GameMsg>& msg, bool skip)
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
				if (!skip)
				{
					t.rotation.Normalize();
					e.AddComponent<comp::Transform>(t);
				}
				break;
			}
			case ecs::Component::VELOCITY:
			{
				comp::Velocity v;
				msg >> v;
				if (!skip)
				{
					e.AddComponent<comp::Velocity>(v);
				}
				break;
			}
			case ecs::Component::MESH_NAME:
			{
				std::string nameString;
				NameType name;
				msg >> name;
				if (!skip)
				{
					switch (name)
					{
					case NameType::MESH_CUBE:
					{
						nameString = "Cube.obj";
						break;
					}
					case NameType::MESH_WATERMILL:
					{
						nameString = "WaterMill.fbx";
						e.AddComponent<comp::Watermill>();
						break;
					}
					case NameType::MESH_WATERMILLHOUSE:
					{
						nameString = "WaterMillHouse.fbx";
						break;
					}
					case NameType::MESH_RUINED_WATERMILLHOUSE:
					{
						nameString = "WaterMillHousePile.fbx";
						break;
					}
					case NameType::MESH_HOUSE5:
					{
						nameString = "House5.fbx";
						break;
					}
					case NameType::MESH_RUINED_HOUSE5:
					{
						nameString = "House5Pile.fbx";
						break;
					}
					case NameType::MESH_HOUSE6:
					{
						nameString = "House6.fbx";
						break;
					}
					case NameType::MESH_RUINED_HOUSE6:
					{
						nameString = "House6Pile.fbx";
						break;
					}
					case NameType::MESH_HOUSE7:
					{
						nameString = "House7.fbx";
						break;
					}
					case NameType::MESH_RUINED_HOUSE7:
					{
						nameString = "House7Pile.fbx";
						break;
					}
					case NameType::MESH_HOUSE8:
					{
						nameString = "House8.fbx";
						break;
					}
					case NameType::MESH_RUINED_HOUSE8:
					{
						nameString = "House8Pile.fbx";
						break;
					}
					case NameType::MESH_HOUSE9:
					{
						nameString = "House9.fbx";
						break;
					}
					case NameType::MESH_RUINED_HOUSE9:
					{
						nameString = "House9Pile.fbx";
						break;
					}
					case NameType::MESH_HOUSE10:
					{
						nameString = "House10.fbx";
						break;
					}
					case NameType::MESH_RUINED_HOUSE10:
					{
						nameString = "House10Pile.fbx";
						break;
					}
					case NameType::MESH_DOOR1:
					{
						nameString = "Door1.fbx";
						break;
					}
					case NameType::MESH_DOOR5:
					{
						nameString = "Door5.fbx";
						break;
					}
					case NameType::MESH_DOOR6:
					{
						nameString = "Door6.fbx";
						break;
					}
					case NameType::MESH_DOOR7:
					{
						nameString = "Door7.fbx";
						break;
					}
					case NameType::MESH_DOOR8:
					{
						nameString = "Door8.fbx";
						break;
					}
					case NameType::MESH_DOOR9:
					{
						nameString = "Door9.fbx";
						break;
					}
					case NameType::MESH_DOOR10:
					{
						nameString = "Door10.fbx";
						break;
					}
					case NameType::MESH_HOUSEROOF:
					{
						nameString = "HouseRoof.fbx";
						break;
					}
					case NameType::MESH_DEFENCE1X1_LVL0:
					{
						nameString = "lvl0_1x1.fbx";
						break;
					}
					case NameType::MESH_DEFENCE1X1_LVL1:
					{
						nameString = "lvl1_1x1.fbx";
						break;
					}
					case NameType::MESH_DEFENCE1X1_LVL2:
					{
						nameString = "lvl2_1x1.fbx";
						break;
					}
					case NameType::MESH_DEFENCE1X1_LVL3:
					{
						nameString = "lvl3_1x1.fbx";
						break;
					}
					case NameType::MESH_DEFENCE1X3_LVL0:
					{
						nameString = "lvl0_1x3.fbx";
						break;
					}
					case NameType::MESH_DEFENCE1X3_LVL1:
					{
						nameString = "lvl1_1x3.fbx";
						break;
					}
					case NameType::MESH_DEFENCE1X3_LVL2:
					{
						nameString = "lvl2_1x3.fbx";
						break;
					}
					case NameType::MESH_DEFENCE1X3_LVL3:
					{
						nameString = "lvl3_1x3.fbx";
						break;
					}
					case NameType::MESH_KNIGHT:
					{
						nameString = "Knight.fbx";
						break;
					}
					case NameType::MESH_MAGE:
					{
						nameString = "Mage.fbx";
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
					case NameType::MESH_VILLAGER:
					{
						nameString = "Villager.fbx";
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
				}
				break;
			}
			case ecs::Component::ANIMATOR_NAME:
			{
				AnimName name;
				msg >> name;
				if (!skip)
				{
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
					case AnimName::ANIM_MAGE:
					{
						nameString = "Mage.anim";
						break;
					}
					case AnimName::ANIM_VILLAGER:
					{
						nameString = "Villager.anim";
						break;
					}
					default:
					{
						nameString = "Knight.anim";
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
				}
				break;
			}
			case ecs::Component::HEALTH:
			{
				comp::Health hp;
				msg >> hp;
				if (!skip)
				{
					Scene& scene = GetScene("Game");
					if (GetCurrentScene() == &scene)
					{
						GameSystems::UpdateHealthbar(this);

						if (e == m_players.at(m_localPID))
						{
							if (!hp.isAlive)
							{
								m_isSpectating = true;
								scene.GetCollection("SpectateUI")->elements[0]->SetVisiblity(true);
								scene.GetCollection("SpectateUI")->elements[1]->SetVisiblity(true);
							}
							else
							{
								if (m_isSpectating)
								{
									scene.GetCollection("SpectateUI")->elements[0]->SetVisiblity(false);
									scene.GetCollection("SpectateUI")->elements[1]->SetVisiblity(false);
									m_isSpectating = false;
									Camera* cam = GetCurrentScene()->GetCurrentCamera();

									cam->SetFollowEntity(m_players.at(m_localPID));
								}
							}
						}
					}
					comp::Health* health = e.GetComponent<comp::Health>();
					comp::Transform* transform = e.GetComponent<comp::Transform>();
					if (health && transform)
					{
						combat_text_inst_t cText;
						// Signal health gain.
						if (health->currentHealth <= hp.currentHealth)
						{
							cText.type = combat_text_enum::HEALTH_GAIN;
							cText.pos = transform->position;
						}
						else if (health->currentHealth > hp.currentHealth)
						{
							cText.type = combat_text_enum::HEALTH_LOSS;
							cText.pos = transform->position;
						}

						cText.timeRendered = omp_get_wtime();
						cText.pos.y += 15;
						cText.end_pos = cText.pos;
						cText.end_pos.y += 50;
						cText.amount = std::abs(health->currentHealth - hp.currentHealth);
						GetScene("Game").PushCombatText(cText);

						if (hp.currentHealth <= 0)
						{
							// Spawn a bloodsplat.
							Entity e = GetScene("Game").CreateEntity();
							e.AddComponent<comp::Decal>(*transform);
						}
					}

					e.AddComponent<comp::Health>(hp);
				}
				break;
			}
			case ecs::Component::BOUNDING_ORIENTED_BOX:
			{
				dx::BoundingOrientedBox box;
				msg >> box;
				if (!skip)
				{
					comp::OrientedBoxCollider* collider = e.AddComponent<comp::OrientedBoxCollider>();
					collider->Center = box.Center;
					collider->Extents = box.Extents;
					collider->Orientation = box.Orientation;
				}
				break;
			}
			case ecs::Component::BOUNDING_SPHERE:
			{
				dx::BoundingSphere s;
				msg >> s;
				if (!skip)
				{
					comp::SphereCollider* collider = e.AddComponent<comp::SphereCollider>();
					collider->Center = s.Center;
					collider->Radius = s.Radius;
				}
				break;
			}
			case ecs::Component::PARTICLEMITTER:
			{
				comp::ParticleEmitter p;
				msg >> p;
				if (!skip)
				{
					e.AddComponent<comp::EmitterParticle>(p.positionOffset, (int)p.nrOfParticles, p.sizeMulitplier, p.type, p.lifeTime, p.speed, p.hasDeathTimer);
				}
				break;
			}
			case ecs::Component::PLAYER:
			{
				comp::Player p;
				msg >> p;
				if (!skip)
				{
					e.AddComponent<comp::Player>(p);
				}

				break;
			}
			case ecs::Component::COST:
			{
				comp::Cost c;
				msg >> c;
				if (!skip)
				{
					e.AddComponent<comp::Cost>(c);
				}
				break;
			}
			case ecs::Component::KD:
			{
				comp::KillDeaths kd;
				msg >> kd;
				e.AddComponent<comp::KillDeaths>(kd);
				break;
			}
			default:
				LOG_WARNING("Retrieved unimplemented component %u", i);
				break;
			}
		}
	}
}

void Game::ChangeSpectatedPlayer()
{
	if (m_isSpectating)
	{
		Camera* cam = GetScene("Game").GetCurrentCamera();
		if (cam->GetCameraType() == CAMERATYPE::PLAY)
		{
			Entity current = cam->GetTargetEntity();

			uint32_t key = current.GetComponent<comp::Network>()->id;
			// START ON CURRENT SPECTATED TARGET
			auto it = m_players.find(key);
			it++;

			// Iterate to next the next player
			for (int i = 0; i < MAX_PLAYERS_PER_LOBBY - 1; i++)
			{
				// Sanity check if we reach end of the list
				if (it == m_players.end())
				{
					it = m_players.begin();
				}
				// Don't spectate yourself you are dead & check if other player is alive
				if (it->first != m_localPID && it->second.GetComponent<comp::Health>()->isAlive)
				{
					// Check so we are not already following this entity
					if (current != it->second)
					{
						cam->SetFollowEntity(it->second);
						break;
					}
				}
				it++;
			}
		}
	}
}

void Game::UpdateInput()
{
	m_inputState.axisHorizontal = InputSystem::Get().GetAxis(Axis::HORIZONTAL);
	m_inputState.axisVertical = InputSystem::Get().GetAxis(Axis::VERTICAL);
	m_inputState.mousewheelDir = InputSystem::Get().GetMouseWheelDirection();

	if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::HELD))
	{
		m_inputState.leftMouse = true;
	}

	if (InputSystem::Get().CheckMouseKey(MouseKey::RIGHT, KeyState::HELD))
	{
		m_inputState.rightMouse = true;
	}

	if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED))
	{
		this->ChangeSpectatedPlayer();
	}

	if (InputSystem::Get().CheckMouseKey(MouseKey::RIGHT, KeyState::PRESSED))
	{
		switch (GetShopItem())
		{
		case ShopItem::Defence1x1:
		{
			break;
		}
		case ShopItem::Defence1x3:
		{
			SetShopItem(ShopItem::None);
			break;
		}
		case ShopItem::None:
		{
			break;
		}
		}
	}
	m_inputState.mouseRay = InputSystem::Get().GetMouseRay();

	if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::LeftShift, KeyState::PRESSED))
	{
		m_inputState.key_shift = true;
	}
}