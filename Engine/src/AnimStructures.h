#pragma once

/*
	Bone/joint information
*/
struct bone_t
{
	std::string name		= "";
	sm::Matrix	inverseBind = {};
	int			parentIndex = -1;
};

/*
	Also holds when the last 
	keyframes was for that bone
*/
struct bone_keyFrames_t
{
	std::string			name		= "";
	sm::Matrix			inverseBind = {};
	int					parentIndex = -1;
	std::array<UINT, 3> lastKeys	= {};
};

enum class EAnimationType
{
	NONE,
	IDLE,
	WALK,
	RUN,
	PRIMARY_ATTACK,
	SECONDARY_ATTACK,
	ABILITY1,
	ABILITY2,
	ABILITY3,
	ABILITY4,
	TAKE_DAMAGE,
	PLACE_DEFENCE
};
