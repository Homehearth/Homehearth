#include "NetServerPCH.h"
#include "HouseManager.h"
#include "ServerSystems.h"
HouseManager::HouseManager()
	:houseColliders(nullptr)
{
}

void HouseManager::InitializeHouses(HeadlessScene& scene, QuadTree* qt)
{
	//Init all the houses for the scene
	Entity house5 = CreateHouse(scene, NameType::MESH_HOUSE5, NameType::MESH_DOOR5, NameType::EMPTY);
	Entity villager1 = VillagerManagement::CreateVillager(scene, house5.GetComponent<comp::House>()->attackNode);
	Entity house6 = CreateHouse(scene, NameType::MESH_HOUSE6, NameType::MESH_DOOR6, NameType::MESH_HOUSEROOF);
	Entity villager2 = VillagerManagement::CreateVillager(scene, house6.GetComponent<comp::House>()->attackNode);
	Entity house7 = CreateHouse(scene, NameType::MESH_HOUSE7, NameType::MESH_DOOR7, NameType::EMPTY);
	Entity villager3 = VillagerManagement::CreateVillager(scene, house7.GetComponent<comp::House>()->attackNode);
	Entity house8 = CreateHouse(scene, NameType::MESH_HOUSE8, NameType::MESH_DOOR8, NameType::EMPTY);
	Entity villager4 = VillagerManagement::CreateVillager(scene, house8.GetComponent<comp::House>()->attackNode);
	Entity house9 = CreateHouse(scene, NameType::MESH_HOUSE9, NameType::MESH_DOOR9, NameType::EMPTY);
	Entity villager5 = VillagerManagement::CreateVillager(scene, house9.GetComponent<comp::House>()->attackNode);
	Entity house10 = CreateHouse(scene, NameType::MESH_HOUSE10, NameType::MESH_DOOR10, NameType::EMPTY);
	Entity villager6 = VillagerManagement::CreateVillager(scene, house10.GetComponent<comp::House>()->attackNode);
	Entity waterMillHouse = CreateHouse(scene, NameType::MESH_WATERMILLHOUSE, NameType::MESH_DOOR1, NameType::EMPTY);
	Entity waterMill = CreateHouse(scene, NameType::MESH_WATERMILL, NameType::EMPTY, NameType::EMPTY);

	waterMill.GetComponent<comp::House>()->isDead = true;

	//Insert houses to quad tree
	qt->Insert(house5);
	qt->Insert(house6);
	qt->Insert(house7);
	qt->Insert(house8);
	qt->Insert(house9);
	qt->Insert(house10);
	qt->Insert(waterMillHouse);
}

void HouseManager::AddCollider(NameType houseType, Entity house) const
{
	comp::House* houseComp = house.GetComponent<comp::House>();

	if (houseType == NameType::MESH_HOUSE5 || houseType == NameType::MESH_RUINED_HOUSE5)
	{
		comp::OrientedBoxCollider* obb = house.AddComponent<comp::OrientedBoxCollider>();
		obb->Center = houseColliders->at("House5_Collider.fbx").Center;
		obb->Extents = houseColliders->at("House5_Collider.fbx").Extents;
		obb->Orientation = houseColliders->at("House5_Collider.fbx").Orientation;
		obb->Center.y = 0.0f;

		if(houseType == NameType::MESH_RUINED_HOUSE5)
		{
			houseComp->attackNode = nullptr;
			houseComp->isDead = true;
		}
		else
		{
			houseComp->attackNode = Blackboard::Get().GetPathFindManager()->GetNodes()[39][18].get();
			houseComp->isDead = false;
		}
	}
	else if (houseType == NameType::MESH_HOUSE6 || houseType == NameType::MESH_RUINED_HOUSE6)
	{
		comp::OrientedBoxCollider* obb = house.AddComponent<comp::OrientedBoxCollider>();
		obb->Center = houseColliders->at("House6_Collider.fbx").Center;
		obb->Extents = houseColliders->at("House6_Collider.fbx").Extents;
		obb->Orientation = houseColliders->at("House6_Collider.fbx").Orientation;
		obb->Center.y = 0.0f;
		if (houseType == NameType::MESH_RUINED_HOUSE6)
		{
			houseComp->attackNode = nullptr;
			houseComp->isDead = true;
		}
		else
		{
			houseComp->attackNode = Blackboard::Get().GetPathFindManager()->GetNodes()[42][27].get();
			houseComp->isDead = false;
		}
	}
	else if (houseType == NameType::MESH_HOUSE7 || houseType == NameType::MESH_RUINED_HOUSE7)
	{
		comp::OrientedBoxCollider* obb = house.AddComponent<comp::OrientedBoxCollider>();
		obb->Center = houseColliders->at("House7_Collider.fbx").Center;
		obb->Extents = houseColliders->at("House7_Collider.fbx").Extents;
		obb->Orientation = houseColliders->at("House7_Collider.fbx").Orientation;
		obb->Center.y = 0.0f;
		if (houseType == NameType::MESH_RUINED_HOUSE7)
		{
			houseComp->attackNode = nullptr;
			houseComp->isDead = true;
		}
		else
		{
			houseComp->attackNode = Blackboard::Get().GetPathFindManager()->GetNodes()[42][22].get();
			houseComp->isDead = false;
		}
	}
	else if (houseType == NameType::MESH_HOUSE8 || houseType == NameType::MESH_RUINED_HOUSE8)
	{
		comp::OrientedBoxCollider* obb = house.AddComponent<comp::OrientedBoxCollider>();
		obb->Center = houseColliders->at("House8_Collider.fbx").Center;
		obb->Extents = houseColliders->at("House8_Collider.fbx").Extents;
		obb->Orientation = houseColliders->at("House8_Collider.fbx").Orientation;
		obb->Center.y = 0.0f;
		if (houseType == NameType::MESH_RUINED_HOUSE8)
		{
			houseComp->attackNode = nullptr;
			houseComp->isDead = true;
		}
		else
		{
			houseComp->attackNode = Blackboard::Get().GetPathFindManager()->GetNodes()[44][42].get();
			houseComp->isDead = false;
		}
	}
	else if (houseType == NameType::MESH_HOUSE9 || houseType == NameType::MESH_RUINED_HOUSE9)
	{
		comp::OrientedBoxCollider* obb = house.AddComponent<comp::OrientedBoxCollider>();
		obb->Center = houseColliders->at("House9_Collider.fbx").Center;
		obb->Extents = houseColliders->at("House9_Collider.fbx").Extents;
		obb->Orientation = houseColliders->at("House9_Collider.fbx").Orientation;
		obb->Center.y = 0.0f;
		if (houseType == NameType::MESH_RUINED_HOUSE9)
		{

			houseComp->attackNode = nullptr;
			houseComp->isDead = true;
		}
		else
		{
			houseComp->attackNode = Blackboard::Get().GetPathFindManager()->GetNodes()[33][22].get();
			houseComp->isDead = false;
		}
	}
	else if (houseType == NameType::MESH_HOUSE10 || houseType == NameType::MESH_RUINED_HOUSE10)
	{
		comp::OrientedBoxCollider* obb = house.AddComponent<comp::OrientedBoxCollider>();
		obb->Center = houseColliders->at("House10_Collider.fbx").Center;
		obb->Extents = houseColliders->at("House10_Collider.fbx").Extents;
		obb->Orientation = houseColliders->at("House10_Collider.fbx").Orientation;
		obb->Center.y = 0.0f;
		if (houseType == NameType::MESH_RUINED_HOUSE10)
		{
			houseComp->attackNode = nullptr;
			houseComp->isDead = true;
		}
		else
		{
			houseComp->attackNode = Blackboard::Get().GetPathFindManager()->GetNodes()[28][34].get();
			houseComp->isDead = false;
		}

	}
	else if (houseType == NameType::MESH_WATERMILLHOUSE || houseType == NameType::MESH_RUINED_WATERMILLHOUSE)
	{
		comp::OrientedBoxCollider* obb = house.AddComponent<comp::OrientedBoxCollider>();
		obb->Center = houseColliders->at("WaterMillHouse_Collider.fbx").Center;
		obb->Extents = houseColliders->at("WaterMillHouse_Collider.fbx").Extents;
		obb->Orientation = houseColliders->at("WaterMillHouse_Collider.fbx").Orientation;
		obb->Center.y = 0.0f;
		if (houseType == NameType::MESH_RUINED_WATERMILLHOUSE)
		{
			houseComp->attackNode = nullptr;
			houseComp->isDead = true;
		}
		else
		{
			houseComp->attackNode = Blackboard::Get().GetPathFindManager()->GetNodes()[43][34].get();
			houseComp->isDead = false;
		}
	}
}

Entity HouseManager::CreateHouse(HeadlessScene& scene, NameType houseType, NameType doorType, NameType roofType)
{
	//Create the new house entity
	Entity houseEntity = scene.CreateEntity();
	houseEntity.AddComponent<comp::MeshName>()->name = houseType;
	houseEntity.AddComponent<comp::Transform>();
	houseEntity.AddComponent<comp::Tag<TagType::STATIC>>();
	houseEntity.AddComponent<comp::Network>();
	houseEntity.AddComponent<comp::Health>();
	comp::House* house = houseEntity.AddComponent<comp::House>();
	house->houseType = houseType;

	//Add door if door type was specified
	if (doorType != NameType::EMPTY)
	{
		Entity door = scene.CreateEntity();
		door.AddComponent<comp::MeshName>()->name = doorType;
		door.AddComponent<comp::Transform>();
		door.AddComponent<comp::Network>();
		house->door = door;
		house->doorType = doorType;
	}

	//Add roof if roof type was specified
	if (roofType != NameType::EMPTY)
	{
		Entity roof = scene.CreateEntity();
		roof.AddComponent<comp::MeshName>()->name = roofType;
		roof.AddComponent<comp::Transform>();
		roof.AddComponent<comp::Network>();
		houseEntity.GetComponent<comp::House>()->houseRoof = roof;
		house->roofType = roofType;
	}

	//Add the correct collider for the specified house type
	AddCollider(houseType, houseEntity);

	//insert in house map for blackboard so (AI can target it) Entity is both key and value
	Houses_t* houses = Blackboard::Get().GetValue<Houses_t>("houses");
	//If no house map is present in blackboard add one
	if(houses == nullptr)
	{
		Houses_t houseMap;
		Blackboard::Get().AddValue<Houses_t>("houses", houseMap);
	}
	houses = Blackboard::Get().GetValue<Houses_t>("houses");
	houses->houses.insert(std::pair<Entity, Entity>(houseEntity, houseEntity));

	return houseEntity;
}

void HouseManager::SetHouseColliders(std::unordered_map<std::string, comp::OrientedBoxCollider>* houseColliders)
{
	this->houseColliders = houseColliders;
}

NameType HouseManager::GetRuinedHouseType(NameType houseType)
{
	if(houseType == NameType::MESH_HOUSE5)
	{
		return NameType::MESH_RUINED_HOUSE5;
	}
	if(houseType == NameType::MESH_HOUSE6)
	{
		return NameType::MESH_RUINED_HOUSE6;
	}
	if (houseType == NameType::MESH_HOUSE7)
	{
		return NameType::MESH_RUINED_HOUSE7;
	}
	if (houseType == NameType::MESH_HOUSE8)
	{
		return NameType::MESH_RUINED_HOUSE8;
	}
	if (houseType == NameType::MESH_HOUSE9)
	{
		return NameType::MESH_RUINED_HOUSE9;
	}
	if (houseType == NameType::MESH_HOUSE10)
	{
		return NameType::MESH_RUINED_HOUSE10;
	}
	if (houseType == NameType::MESH_WATERMILLHOUSE)
	{
		return NameType::MESH_RUINED_WATERMILLHOUSE;
	}

	return NameType::EMPTY;
}
