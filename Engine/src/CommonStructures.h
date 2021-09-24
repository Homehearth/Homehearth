#pragma once

/*
	Basic objects like static meshes
	//[TODO] Discuss bitanget?
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
	Uses 3 bones per vertex which is cheaper than 4 
	without much of a noticeable difference
*/
struct anim_vertex_t
{
	simple_vertex_t vertex	   = {};
	dx::XMUINT3		boneID	   = {};
	sm::Vector3		boneWeight = {};
};


struct basic_model_matrix_t
{
	sm::Matrix worldMatrix;
};


struct camera_Matrix_t
{
	sm::Matrix projection;
	sm::Matrix view;
};