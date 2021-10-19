#pragma once

struct Ray_t
{
	sm::Vector3 rayPos;
	sm::Vector3 rayDir;
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
	Debugging with collision boxes
*/
struct debug_vertex_t
{
	sm::Vector3 position = {};
	//sm::Vector3 color = {}; could be implemented if needed
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

	// inverseProjection
	// screenDimensions
};

ALIGN16
struct light_t
{
	sm::Vector4 position	= {};
	sm::Vector4 direction	= {};
	sm::Vector4 color		= {};
	float		attenuation = 0.0f;	// Point has Quadratic while Directional has Constant - assimp 
	UINT		type		= 1;	// 0 = Directional, 1 = Point
	UINT		enabled		= 1;	// 0 = Off, 1 = On
	float		range		= 0.0f;	// Not available from assimp
};

/*
	The number of different lights used in the scene.
 */
ALIGN16
struct light_counts_t
{
	uint32_t NumPointLights;
	uint32_t NumDirectionalLights;
};

/*
	Grid frustums for light culling.
 */
ALIGN16
struct frustum_t
{
	sm::Vector4 Planes[4];
};

/*
	Dispatch params used for compute shaders.
 */
ALIGN16
struct dispatch_params_t
{
	dx::XMUINT3 NumThreadGroups;
	dx::XMUINT3 NumThreads;
};

