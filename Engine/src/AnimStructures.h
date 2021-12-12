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
	ABILITY1,
	ABILITY2,
	ABILITY3,
	ABILITY4,
	TAKE_DAMAGE,
	PLACE_DEFENCE,
	DEAD
};

struct animstate_t
{
	EAnimationType from;
	EAnimationType to;

	animstate_t(EAnimationType x, EAnimationType y)
	{
		this->from = x;
		this->to = y;
	}
	bool operator==(const animstate_t& state) const
	{
		return from == state.from && to == state.to;
	}
};

struct animstate_hash_fn
{
	std::size_t operator() (const animstate_t& state) const
	{
		std::size_t h1 = std::hash<EAnimationType>()(state.from);
		std::size_t h2 = std::hash<EAnimationType>()(state.to);
		return h1 ^ h2;
	}
};

struct animstateInfo
{
	double		blendTimer		= 0;		//How long we have blended so far
	double		blendDuration	= 0;		//How long to blend
	bool		animationCancel = false;	//Possible to cancel animation
	std::string devidebone		= "";		//With an upper animation this is the bone to start at
};

enum class EAnimStatus
{
	NONE,
	ONE_ANIM,
	TWO_ANIM_BLEND,
	TWO_ANIM_UPPER_LOWER,
	THREE_ANIM_UPPER_LOWER_BLEND
};
