#pragma once

/*
	Bone/joint information
*/
struct bone_t
{
	std::string name = "";
	sm::Matrix	inverseBind = {};
	int			parentIndex = -1;
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