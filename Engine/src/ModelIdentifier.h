#pragma once

enum class ModelID : uint16_t
{
	HOUSE5,
	HOUSE6,
	HOUSE7,
	HOUSE8,
	HOUSE9,
	HOUSE10,
	HOUSEROOF,
	TREE3,
	TREE5,
	TREE6,
	TREE8,
	WATER
};

const std::string House5		= "House5";
const std::string House6		= "House6";
const std::string House7		= "House7";
const std::string House8		= "House8";
const std::string House9		= "House9";
const std::string House10		= "House10";
const std::string HouseRoof		= "HouseRoof";
const std::string Tree3			= "Tree3";
const std::string Tree5			= "Tree5";
const std::string Tree6			= "Tree6";
const std::string Tree8			= "Tree8";
const std::string Door5			= "Door5";
const std::string Door6			= "Door6";
const std::string Door7			= "Door7";
const std::string Door8			= "Door8";
const std::string Door9			= "Door9";
//const std::string Water			= "WaterMesh";

//non transparent
const std::string WaterEdge     = "WaterEdgeMesh";
const std::string WaterFloor    = "WaterFloorMesh";
const std::string Door10		= "Door10";

enum class AnimName : uint16_t
{
	ANIM_NONE,
	ANIM_KNIGHT,
	ANIM_MAGE,
	ANIM_MONSTER,
	ANIM_VILLAGER
};

enum class NameType : uint32_t
{
	EMPTY,
	MESH_CUBE,
	MESH_DEFENCE,
	MESH_KNIGHT,
	MESH_VILLAGER,
	MESH_MAGE,
	MESH_MONSTER,
	MESH_SPHERE,
	MESH_HOUSE5,
	MESH_RUINED_HOUSE5,
	MESH_HOUSE6,
	MESH_RUINED_HOUSE6,
	MESH_HOUSE7,
	MESH_RUINED_HOUSE7,
	MESH_HOUSE8,
	MESH_RUINED_HOUSE8,
	MESH_HOUSE9,
	MESH_RUINED_HOUSE9,
	MESH_HOUSE10,
	MESH_RUINED_HOUSE10,
	MESH_HOUSEROOF,
	MESH_WATERMILL,
	MESH_WATERMILLHOUSE,
	MESH_RUINED_WATERMILLHOUSE,
	MESH_DOOR1,
	MESH_DOOR5,
	MESH_DOOR6,
	MESH_DOOR7,
	MESH_DOOR8,
	MESH_DOOR9,
	MESH_DOOR10,
	MESH_DEFENCE1X1_LVL0,
	MESH_DEFENCE1X1_LVL1,
	MESH_DEFENCE1X1_LVL2,
	MESH_DEFENCE1X1_LVL3,
	MESH_DEFENCE1X3_LVL0,
	MESH_DEFENCE1X3_LVL1,
	MESH_DEFENCE1X3_LVL2,
	MESH_DEFENCE1X3_LVL3,
};