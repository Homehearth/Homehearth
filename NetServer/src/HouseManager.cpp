#include "NetServerPCH.h"
#include "HouseManager.h"

HouseManager::HouseManager()
	:houseColliders(nullptr)
{
}

void HouseManager::InitializeHouses(HeadlessScene& scene, QuadTree* qt)
{
	//House 5
	Entity house5 = CreateHouse(scene, NameType::MESH_HOUSE5, NameType::MESH_DOOR5, NameType::EMPTY);
	Entity house6 = CreateHouse(scene, NameType::MESH_HOUSE6, NameType::MESH_DOOR6, NameType::MESH_HOUSEROOF);
	Entity house7 = CreateHouse(scene, NameType::MESH_HOUSE7, NameType::MESH_DOOR7, NameType::EMPTY);
	Entity house8 = CreateHouse(scene, NameType::MESH_HOUSE8, NameType::MESH_DOOR8, NameType::EMPTY);
	Entity house9 = CreateHouse(scene, NameType::MESH_HOUSE9, NameType::MESH_DOOR9, NameType::EMPTY);
	Entity house10 = CreateHouse(scene, NameType::MESH_HOUSE10, NameType::MESH_DOOR10, NameType::EMPTY);
	Entity waterMillHouse = CreateHouse(scene, NameType::MESH_WATERMILLHOUSE, NameType::MESH_DOOR1, NameType::EMPTY);
	Entity waterMill = CreateHouse(scene, NameType::MESH_WATERMILL, NameType::EMPTY, NameType::EMPTY);
	//Insert houses to quad tree
	qt->Insert(house5);
	qt->Insert(house6);
	qt->Insert(house7);
	qt->Insert(house8);
	qt->Insert(house9);
	qt->Insert(house10);
	qt->Insert(waterMillHouse);
}

void HouseManager::AddCollider(NameType houseType, Entity house)
{
	if (houseType == NameType::MESH_HOUSE5 || houseType == NameType::MESH_RUINED_HOUSE5)
	{
		comp::OrientedBoxCollider* obb = house.AddComponent<comp::OrientedBoxCollider>();
		obb->Center = houseColliders->at("House5_Collider.fbx").Center;
		obb->Extents = houseColliders->at("House5_Collider.fbx").Extents;
		obb->Orientation = houseColliders->at("House5_Collider.fbx").Orientation;
	}
	else if (houseType == NameType::MESH_HOUSE6 || houseType == NameType::MESH_RUINED_HOUSE6)
	{
		comp::OrientedBoxCollider* obb = house.AddComponent<comp::OrientedBoxCollider>();
		obb->Center = houseColliders->at("House6_Collider.fbx").Center;
		obb->Extents = houseColliders->at("House6_Collider.fbx").Extents;
		obb->Orientation = houseColliders->at("House6_Collider.fbx").Orientation;
	}
	else if (houseType == NameType::MESH_HOUSE7 || houseType == NameType::MESH_RUINED_HOUSE7)
	{
		comp::OrientedBoxCollider* obb = house.AddComponent<comp::OrientedBoxCollider>();
		obb->Center = houseColliders->at("House7_Collider.fbx").Center;
		obb->Extents = houseColliders->at("House7_Collider.fbx").Extents;
		obb->Orientation = houseColliders->at("House7_Collider.fbx").Orientation;
	}
	else if (houseType == NameType::MESH_HOUSE8 || houseType == NameType::MESH_RUINED_HOUSE8)
	{
		comp::OrientedBoxCollider* obb = house.AddComponent<comp::OrientedBoxCollider>();
		obb->Center = houseColliders->at("House8_Collider.fbx").Center;
		obb->Extents = houseColliders->at("House8_Collider.fbx").Extents;
		obb->Orientation = houseColliders->at("House8_Collider.fbx").Orientation;
	}
	else if (houseType == NameType::MESH_HOUSE9 || houseType == NameType::MESH_RUINED_HOUSE9)
	{
		comp::OrientedBoxCollider* obb = house.AddComponent<comp::OrientedBoxCollider>();
		obb->Center = houseColliders->at("House9_Collider.fbx").Center;
		obb->Extents = houseColliders->at("House9_Collider.fbx").Extents;
		obb->Orientation = houseColliders->at("House9_Collider.fbx").Orientation;
	}
	else if (houseType == NameType::MESH_HOUSE10 || houseType == NameType::MESH_RUINED_HOUSE10)
	{
		comp::OrientedBoxCollider* obb = house.AddComponent<comp::OrientedBoxCollider>();
		obb->Center = houseColliders->at("House10_Collider.fbx").Center;
		obb->Extents = houseColliders->at("House10_Collider.fbx").Extents;
		obb->Orientation = houseColliders->at("House10_Collider.fbx").Orientation;
	}
	else if (houseType == NameType::MESH_WATERMILLHOUSE || houseType == NameType::MESH_RUINED_WATERMILLHOUSE)
	{
		comp::OrientedBoxCollider* obb = house.AddComponent<comp::OrientedBoxCollider>();
		obb->Center = houseColliders->at("WaterMillHouse_Collider.fbx").Center;
		obb->Extents = houseColliders->at("WaterMillHouse_Collider.fbx").Extents;
		obb->Orientation = houseColliders->at("WaterMillHouse_Collider.fbx").Orientation;
	}
}

Entity HouseManager::CreateHouse(HeadlessScene& scene, NameType houseType, NameType doorType, NameType roofType)
{
	Entity houseEntity = scene.CreateEntity();
	houseEntity.AddComponent<comp::MeshName>()->name = houseType;
	houseEntity.AddComponent<comp::Transform>();
	houseEntity.AddComponent<comp::Tag<TagType::STATIC>>();
	houseEntity.AddComponent<comp::Network>();
	houseEntity.AddComponent<comp::Health>();

	comp::House* house = houseEntity.AddComponent<comp::House>();
	house->houseType = houseType;

	if (doorType != NameType::EMPTY)
	{
		Entity door = scene.CreateEntity();
		door.AddComponent<comp::MeshName>()->name = doorType;
		door.AddComponent<comp::Transform>();
		door.AddComponent<comp::Network>();
		house->door = door;
		house->doorType = doorType;
	}

	if (roofType != NameType::EMPTY)
	{
		Entity roof = scene.CreateEntity();
		roof.AddComponent<comp::MeshName>()->name = roofType;
		roof.AddComponent<comp::Transform>();
		roof.AddComponent<comp::Network>();
		houseEntity.GetComponent<comp::House>()->houseRoof = roof;
		house->roofType = roofType;
	}
	comp::OrientedBoxCollider* obb;

	AddCollider(houseType, houseEntity);

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
