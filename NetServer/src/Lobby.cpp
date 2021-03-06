#include "NetServerPCH.h"
#include "Lobby.h"
#include "Simulation.h"

void Lobby::CreatePlayerEntity(uint32_t playerID, const std::string& name)
{
	// Create Player entity in Game scene
	Entity player = m_simRef->GetGameScene()->CreateEntity();

	comp::Player* playerComp = player.AddComponent<comp::Player>();
	playerComp->playerType = m_playerTypes.top();
	m_playerTypes.pop();
	memcpy(playerComp->name, name.c_str(), name.length());
	player.AddComponent<comp::Network>(playerID);

	m_simRef->ResetPlayer(player);
	m_players[playerID] = player;
}

std::unordered_map<uint32_t, Entity>::iterator Lobby::RemovePlayer(std::unordered_map<uint32_t, Entity>::iterator playerIterator)
{
	Entity player = playerIterator->second;

	comp::Player* p = player.GetComponent<comp::Player>();

	if (p)
	{
		m_playerTypes.push(p->playerType);
	}

	uint32_t playerID = playerIterator->first;
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

Lobby::Lobby()
{
	m_isActive = false;
	m_simRef = nullptr;
}

Lobby::~Lobby()
{
}

void Lobby::Init(Simulation* sim)
{
	this->m_simRef = sim;
	this->m_isActive = true;

	m_playerTypes.push(comp::Player::PlayerType::PLAYER_ONE);
	m_playerTypes.push(comp::Player::PlayerType::PLAYER_TWO);
	m_playerTypes.push(comp::Player::PlayerType::PLAYER_THREE);
	m_playerTypes.push(comp::Player::PlayerType::PLAYER_FOUR);
}

void Lobby::AddPlayer(uint32_t gameID, uint32_t playerID, const std::string& name)
{
	message<GameMsg> msg;
	if (!m_isActive || m_players.size() >= MAX_PLAYERS_PER_LOBBY)
	{
		msg.header.id = GameMsg::Lobby_Invalid;
		msg << std::string("Request denied!");
		m_simRef->SendMsg(playerID, msg);

		return;
	}

	msg.header.id = GameMsg::Lobby_Accepted;
	msg << gameID;
	m_simRef->SendMsg(playerID, msg);
	LOG_INFO("Player %d joined lobby: %d", playerID, gameID);
	m_simRef->SendAllEntitiesToPlayer(playerID);
	this->CreatePlayerEntity(playerID, name);
}

void Lobby::Update()
{
	message<GameMsg> msg;
	msg.header.id = GameMsg::Lobby_Update;
	auto it = m_players.begin();

	while (it != m_players.end())
	{
		msg << it->first;
		msg << it->second.GetComponent<comp::Player>()->name;
		msg << it->second.GetComponent<comp::Player>()->classType;
		msg << it->second.GetComponent<comp::Player>()->isReady;
		it++;
	}

	msg << static_cast<uint8_t>(m_players.size());

	m_simRef->BroadcastUDP(msg);
}

bool Lobby::IsActive() const
{
	return m_isActive;
}

void Lobby::SetActive(bool active)
{
	m_isActive = active;
}

void Lobby::ScanForDisconnects()
{
	auto it = m_players.begin();

	while (it != m_players.end())
	{
		if (!m_simRef->IsPlayerConnected(it->first))
		{
			it = this->RemovePlayer(it);
		}
		else
		{
			it++;
		}
	}
}

Entity Lobby::GetPlayer(uint32_t playerID)const
{
	// Doesnt exist
	if (m_players.find(playerID) == m_players.end())
	{
		return Entity();
	}

	return m_players.at(playerID);
}

bool Lobby::ReadyCheck(uint32_t playerID)
{
	if (m_isActive)
	{
		if (m_players.find(playerID) != m_players.end())
		{
			comp::Player* player = m_players.at(playerID).GetComponent<comp::Player>();
			player->isReady = !player->isReady;

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
				return true;
			}
		}
	}

	return false;
}

bool Lobby::IsEmpty() const
{
	if (m_players.size() == 0)
	{
		return true;
	}

	return false;
}

void Lobby::RemovePlayer(uint32_t playerID)
{
	comp::Player* p = m_players.at(playerID).GetComponent<comp::Player>();

	if (p)
	{
		m_playerTypes.push(p->playerType);
	}

	m_players.at(playerID).Destroy();
	m_players.erase(playerID);

	LOG_INFO("Removed player %u from scene", playerID);
}

void Lobby::Clear()
{
	auto it = m_players.begin();

	while (it != m_players.end())
	{
		comp::Player* p = it->second.GetComponent<comp::Player>();

		if (p)
		{
			m_playerTypes.push(p->playerType);
		}

		LOG_INFO("Removed player %u from scene", it->first);

		it->second.Destroy();
		it = m_players.erase(it);
	}

	m_players.clear();
}
