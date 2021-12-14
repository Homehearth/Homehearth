#pragma once
class Simulation;

class IShop
{
private:

	Simulation* m_sim = nullptr;

public:
	IShop(Simulation* simRef);
	void UseShop(const ShopItem& whatToBuy, const uint32_t& player);
};