#pragma once
class HouseManager
{
public:
	HouseManager();
	~HouseManager() = default;
	void InitializeHouses(HeadlessScene& scene, QuadTree* qt);
	Entity CreateHouse(HeadlessScene& scene, NameType houseType, NameType doorType, NameType roofType);
	void SetHouseColliders(std::unordered_map<std::string, comp::OrientedBoxCollider>* houseColliders);
	NameType GetRuinedHouseType(NameType houseType);
	void SetBlackboard(Blackboard* blackboard);

private:
	std::unordered_map<std::string, comp::OrientedBoxCollider>* houseColliders{};
	void AddCollider(NameType houseType, Entity entity) const;
	Blackboard* blackboard;
	float m_houseHealth = 250;
};

