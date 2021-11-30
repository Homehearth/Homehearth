#include "NetServerPCH.h"
#include "Simulation.h"
#include "IShop.h"

void IShop::SetSimulation(Simulation* sim)
{
	m_sim = sim;
}

void IShop::UseShop(const ShopItem& whatToBuy, const uint32_t& player)
{
	if (m_sim->m_timeCycler.GetTimePeriod() == Cycle::DAY)
	{
		switch (whatToBuy)
		{
		case ShopItem::Primary_Upgrade:
		{
			if (m_sim->GetPlayer(player))
			{
				// Upgrade if melee.
				comp::MeleeAttackAbility* m = m_sim->GetPlayer(player).GetComponent<comp::MeleeAttackAbility>();
				if (m && m_sim->GetCurrency() >= 10)
				{
					m->attackDamage += .5f;
					m_sim->GetCurrency() -= 10;
					m_sim->GetCurrency().hasUpdated = true;
				}

				// Upgrade if ranged.
				comp::RangeAttackAbility* r = m_sim->GetPlayer(player).GetComponent<comp::RangeAttackAbility>();
				if (r && m_sim->GetCurrency() >= 10)
				{
					r->attackDamage += .5f;
					m_sim->GetCurrency() -= 10;
					m_sim->GetCurrency().hasUpdated = true;
				}
			}
			break;
		}
		case ShopItem::Tower_Upgrade:
		{
			if (m_sim->GetCurrency() < 20)
				break;

			/*Upgrade a tower or ALL towers?*/
			//m_pCurrentScene->ForEachComponent<comp::Health, comp::Tag<TagType::STATIC>>([&](comp::Health& h, comp::Tag<TagType::STATIC>& t) {

			//	h.maxHealth += 20;
			//	h.currentHealth += 20;

			//	});

			m_sim->GetCurrency() -= 20;

			break;
		}
		case ShopItem::Heal:
		{
			if (m_sim->GetCurrency() < 5)
				break;

			if (m_sim->GetPlayer(player))
			{
				comp::Health* h = m_sim->GetPlayer(player).GetComponent<comp::Health>();
				if (h)
				{
					h->currentHealth = h->maxHealth;
				}
			}

			m_sim->GetCurrency() -= 5;
			break;
		}
		case ShopItem::Long_Tower:
		{
			if (m_sim->GetPlayer(player))
			{
				m_sim->GetPlayer(player).GetComponent<comp::Player>()->towerSelected = EDefenceType::LARGE;
			}
			break;
		}
		case ShopItem::Short_Tower:
		{
			if (m_sim->GetPlayer(player))
			{
				m_sim->GetPlayer(player).GetComponent<comp::Player>()->towerSelected = EDefenceType::SMALL;
			}
			break;
		}
		default:
		{
			break;
		}
		}
	}
}
