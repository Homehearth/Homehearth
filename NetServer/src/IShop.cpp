#include "NetServerPCH.h"
#include "Simulation.h"
#include "IShop.h"


IShop::IShop(Simulation* simRef)
{
	m_sim = simRef;
}

void IShop::UseShop(const ShopItem& whatToBuy, const uint32_t& player)
{


	audio_t audio = {

		ESoundEvent::NONE,
		sm::Vector3(0.f,0.f,0.f),
			1.0f,
		1000.f,
			false,
			false,
			false,
			false,
			};

		switch (whatToBuy)
		{
		case ShopItem::Primary_Upgrade:
		{
			
			int cost = 300;
			// Upgrade if melee.

			if(m_sim->GetCurrency() < cost)
			{
				audio.type = ESoundEvent::Player_OnCantBuy;

				m_sim->GetGameScene()->ForEachComponent<comp::Player>([&](Entity& playerEntity, comp::Player& player)
					{
						playerEntity.GetComponent<comp::AudioState>()->data.emplace(audio);
					});
				break;
			}

			comp::MeleeAttackAbility* m = m_sim->GetPlayer(player).GetComponent<comp::MeleeAttackAbility>();
			if (m && m_sim->GetCurrency() >= cost && m->upgradeLevel <= 2)
			{
				m->attackDamage *= 1.5f;
				m_sim->GetCurrency() -= cost;
				m->upgradeLevel++;

				audio.type = ESoundEvent::Player_OnBuy;

				m_sim->GetGameScene()->ForEachComponent<comp::Player>([&](Entity& playerEntity, comp::Player& player)
					{
						playerEntity.GetComponent<comp::AudioState>()->data.emplace(audio);
					});
			
			}

			// Upgrade if ranged.
			comp::RangeAttackAbility* r = m_sim->GetPlayer(player).GetComponent<comp::RangeAttackAbility>();
			if (r && m_sim->GetCurrency() >= cost && r->upgradeLevel <= 2)
			{
				r->attackDamage *= 1.5f;
				m_sim->GetCurrency() -= cost;
				r->upgradeLevel++;


				audio.type = ESoundEvent::Player_OnBuy;

				m_sim->GetGameScene()->ForEachComponent<comp::Player>([&](Entity& playerEntity, comp::Player& player)
					{
						playerEntity.GetComponent<comp::AudioState>()->data.emplace(audio);
					});

			}
			break;
		}
		case ShopItem::Heal:
		{
			int cost = 150;
			if (m_sim->GetCurrency() < cost)
			{
				audio.type = ESoundEvent::Player_OnCantBuy;

				m_sim->GetGameScene()->ForEachComponent<comp::Player>([&](Entity& playerEntity, comp::Player& player)
					{
						playerEntity.GetComponent<comp::AudioState>()->data.emplace(audio);
					});
				break;
			}
				

			comp::Health* h = m_sim->GetPlayer(player).GetComponent<comp::Health>();
			if (h && h->upgradeLevel <= 2)
			{
				if (h->currentHealth == h->maxHealth)
				{
					h->maxHealth += 25;
					h->currentHealth = h->maxHealth;
				}
				else
					h->maxHealth += 25;
				
				h->upgradeLevel++;
				m_sim->GetCurrency() -= cost;
				m_sim->GetGameScene()->publish<EComponentUpdated>(m_sim->GetPlayer(player), ecs::Component::HEALTH);

				audio.type = ESoundEvent::Player_OnBuy;

				m_sim->GetGameScene()->ForEachComponent<comp::Player>([&](Entity& playerEntity, comp::Player& player)
					{
						playerEntity.GetComponent<comp::AudioState>()->data.emplace(audio);
					});

			}
			break;
		}
		default:
		{
			break;
		}
		}
}
