#pragma once
#include "RTexture.h"
#include <assimp/material.h>

/*
	---MTL-Standard---
	make sure that the obj-file has a link to material
	relative to where it is, otherwise assimp will not find it 
	* "mtllib ../Materials/Barrel.mtl"

	---PBR---
	Assimp can't load in pbr-material for mtl-files
	Have to use following for it to work:

	* Albedo				"map_Kd Object_Albedo.png"
	* Normal				"map_Kn Object_Normal.png"
	* Metalness				"map_ns Object_Metalness.png"
	* Roughness				"map_Ks Object_Roughness.png"
	* Ambient occlusion		"map_Ka Object_AO.png"
	
*/

class RMaterial : public resource::GResource
{
private:
	enum class ETextureType
	{
		albedo,
		normal,
		metalness,
		roughness,
		ambientOcclusion,
		length
	};

	/*
		Constantbuffer with constants for the material
	*/
	struct matConstants_t
	{
		sm::Vector3	 ambient	= {};		//12 bytes
		float		 shiniess	= 0.0f;		//4 bytes
		sm::Vector3	 diffuse	= {};		//12 bytes
		float		 opacity	= 1.0f;		//4 bytes
		sm::Vector3	 specular	= {};		//12 bytes
		float		 padding	= 0.0f;		//4 bytes
	};
	ComPtr<ID3D11Buffer> m_matConstCB;

	/*
		Constantbuffer that should make it easier in HLSL-shader
		to check if a specific texture exist
	*/
	struct textures_t
	{
		bool	hasAlbedo	 = false;	//1 byte
		bool	hasNormal	 = false;	//1 byte
		bool	hasMetalness = false;	//1 byte
		bool	hasRoughness = false;	//1 byte
		bool	hasAoMap	 = false;	//1 byte
		bool	padding[11]	 = {false};	//11 bytes
	};
	ComPtr<ID3D11Buffer> m_hasTextureCB;

	/*
		All the textures that material has
	*/
	std::shared_ptr<RTexture> m_textures[UINT(ETextureType::length)];

private:
	//Split the path to only get the filename
	std::string GetFilename(const std::string& path);
	bool CreateConstBuf(const matConstants_t& mat);
	bool CreateConstBuf(const textures_t& mat);


public:
	RMaterial();
	~RMaterial();

	void BindMaterial();
	void UnBindMaterial();

	//Loaded from assimp
	bool Create(aiMaterial* aiMat);

	// Inherited via GResource
	// Load material with file - mtl files
	virtual bool Create(const std::string& filename) override;

};
