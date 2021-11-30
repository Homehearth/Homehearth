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
	TREE2,
	TREE3,
	TREE5,
	TREE6,
	TREE8
};

const std::string House5		= "House5";
const std::string House6		= "House6";
const std::string House7		= "House7";
const std::string House8		= "House8";
const std::string House9		= "House9";
const std::string House10		= "House10";
const std::string HouseRoof		= "HouseRoof";
const std::string Tree2			= "Tree2";
const std::string Tree3			= "Tree3";
const std::string Tree5			= "Tree5";
const std::string Tree6			= "Tree6";
const std::string Tree8			= "Tree8";
const std::string Door5			= "Door5";
const std::string Door6			= "Door6";
const std::string Door7			= "Door7";
const std::string Door8			= "Door8";
const std::string Door9			= "Door9";
const std::string Door10		= "Door10";

enum class AnimName : uint16_t
{
	ANIM_NONE,
	ANIM_KNIGHT,
	ANIM_MONSTER
};

enum class NameType : uint32_t
{
	MESH_DEFENCE1X1,
	MESH_DEFENCE1X3,
	MESH_KNIGHT,
	MESH_MONSTER,
	MESH_SPHERE
};