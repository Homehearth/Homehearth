#pragma once

enum class TypeLight : UINT
{
	DIRECTIONAL,
	POINT
};

struct Ray_t
{
	sm::Vector3 rayPos, rayDir;
};

/*
	Basic objects like static meshes
*/
ALIGN16
struct simple_vertex_t
{
	sm::Vector3 position = {};
	sm::Vector2 uv		 = {};
	sm::Vector3 normal	 = {};
	sm::Vector3 tangent  = {};
	sm::Vector3 bitanget = {};
};

/*
	Skeletal animated model.
	Uses 4 bones per vertex for now.
	If optimization is needed, consider lowering to 3.
*/
ALIGN16
struct anim_vertex_t
{
	simple_vertex_t vertex	    = {};
	dx::XMUINT4		boneIDs	    = {};
	sm::Vector4		boneWeights = {};
};

ALIGN16
struct basic_model_matrix_t
{
	sm::Matrix worldMatrix;
};

ALIGN16
struct collider_hit_t
{
	int hit;
};

ALIGN16
struct camera_Matrix_t
{
	sm::Vector4 position;
	sm::Vector4 target;

	sm::Matrix projection;
	sm::Matrix view;
};

ALIGN16
struct light_t
{
	sm::Vector4 position	= {};	//Only in use on Point Lights
	sm::Vector4 direction	= {};	//Only in use on Directional Lights
	sm::Vector4 color		= {};	//Color and Intensity of the Lamp
	float		range		= 0;	//Only in use on Point Lights
	TypeLight	type		= TypeLight::DIRECTIONAL;	// 0 = Directional, 1 = Point
	UINT		enabled		= 0;	// 0 = Off, 1 = On
	float		padding		= 0;
};