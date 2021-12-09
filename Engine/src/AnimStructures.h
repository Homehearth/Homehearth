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

struct lastKeys_t
{
	UINT keys[3] = { 0,0,0 };
};

enum class EAnimationType : uint32_t
{
	NONE,
	IDLE,
	MOVE,
	PRIMARY_ATTACK,
	SECONDARY_ATTACK,
	MOVE_ABILITY,
	ABILITY1,
	ABILITY2,
	ABILITY3,
	ABILITY4,
	TAKE_DAMAGE,
	PLACE_DEFENCE,
	DEAD
};

struct blendstate_t
{
	EAnimationType from;
	EAnimationType to;

	blendstate_t(EAnimationType x, EAnimationType y)
	{
		this->from = x;
		this->to = y;
	}
	bool operator==(const blendstate_t& state) const
	{
		return from == state.from && to == state.to;
	}
};

struct blend_hash_fn
{
	std::size_t operator() (const blendstate_t& state) const
	{
		std::size_t h1 = std::hash<EAnimationType>()(state.from);
		std::size_t h2 = std::hash<EAnimationType>()(state.to);
		return h1 ^ h2;
	}
};