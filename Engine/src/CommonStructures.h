#pragma once

struct Ray_t
{
	sm::Vector3 rayPos, rayDir;
};

/*
	Basic objects like static meshes
	//[TODO] Discuss bitanget?
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
	Skeletal animated meshes
	Uses 3 bones per vertex which is cheaper than 4 
	without much of a noticeable difference
*/
ALIGN16
struct anim_vertex_t
{
	simple_vertex_t vertex	   = {};
	dx::XMUINT3		boneID	   = {};
	sm::Vector3		boneWeight = {};
};

ALIGN16
struct basic_model_matrix_t
{
	sm::Matrix worldMatrix;
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
	sm::Vector4 position;
	sm::Vector4 direction;
	sm::Vector4 color;
	float		range;
	UINT		type;	 // 0 = Directional, 1 = Point
	UINT		enabled; // 0 = Off, 1 = On
};