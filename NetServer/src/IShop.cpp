#include "NetServerPCH.h"
#include "Simulation.h"
#include "IShop.h"

void IShop::SetSimulation(Simulation* sim)
{
	m_sim = sim;
}

void IShop::UseShop(const ShopItem& whatToBuy, const uint32_t& player)
{
	if (m_sim->m_timeCycler.GetTimePeriod() == CyclePeriod::DAY)
	{
		switch (whatToBuy)
		{
		case ShopItem::Primary_Upgrade:
		{
			if (m_sim->GetPlayer(player))
			{
				// Upgrade if melee.
				comp::MeleeAttackAbility* m = m_sim->GetPlayer(player).GetComponent<comp::MeleeAttackAbility>();
				if (m && m_sim->GetCurrency() >= 100 && m->upgradeLevel <= 2)
				{
					m->attackDamage *= 1.5f;
					m_sim->GetCurrency() -= 100;
					m->upgradeLevel++;
				}

				// Upgrade if ranged.
				comp::RangeAttackAbility* r = m_sim->GetPlayer(player).GetComponent<comp::RangeAttackAbility>();
				if (r && m_sim->GetCurrency() >= 100 && r->upgradeLevel <= 2)
				{
					r->attackDamage *= 1.5f;
					m_sim->GetCurrency() -= 100;
					r->upgradeLevel++;
				}
			}
			break;
		}
		case ShopItem::Heal:
		{
			if (m_sim->GetCurrency() < 50)
				break;

			if (m_sim->GetPlayer(player))
			{
				comp::Health* h = m_sim->GetPlayer(player).GetComponent<comp::Health>();
				if (h && h->upgradeLevel <= 2)
				{
					h->maxHealth += 25;
					h->upgradeLevel++;
					m_sim->GetCurrency() -= 50;
				}

				m_sim->GetPlayer(player).UpdateNetwork();
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
