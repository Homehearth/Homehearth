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
	std::string name		= "";
	sm::Matrix	inverseBind = {};
	int			parentIndex = -1;
	UINT		lastKeys[3] = {0};
};

/*
	Keyframes structures
*/
struct positionKey_t
{
	double		time;
	sm::Vector3 val;
};
struct scaleKey_t
{
	double		time;
	sm::Vector3	val;
};
struct rotationKey_t
{
	double			time;
	sm::Quaternion	val;
};