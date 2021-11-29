#pragma once
class HouseManager
{
public:
	HouseManager() = default;
	~HouseManager() = default;
	void InitializeHouses(HeadlessScene& scene, QuadTree* qt);
	Entity CreateHouse(HeadlessScene& scene, NameType houseType, NameType doorType, NameType roofType);

private:


};

