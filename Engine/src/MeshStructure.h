#pragma once
#include "RTexture.h"

namespace dx = DirectX;
namespace sm = DirectX::SimpleMath;

/*
	Basic objects like static meshes
	//TODO: Discuss bitanget?
*/
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
	Uses 3 bones per vertex - cheaper than 4 bones and not much of a noticeable difference
*/
struct anim_vertex_t
{
	simple_vertex_t vertex	   = {};
	dx::XMUINT3		boneID	   = {};
	sm::Vector3		boneWeight = {};
};

enum class ETextureType
{
	diffuse,
	normal,
	metalness,
	roughness
};

/*
	Basic material
*/
struct material_t
{
	sm::Vector3 ambient	 = {};
	sm::Vector3 diffuse  = {};
	sm::Vector3 specular = {};
	float       shiniess = 0.0f;
	float		opacity  = 1.0f;  //?
	std::unordered_map<ETextureType, RTexture*> textures;
};