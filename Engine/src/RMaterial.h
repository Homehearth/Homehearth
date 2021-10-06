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

	* Albedo				"map_Kd   Object_Albedo.png"
	* Normal				"map_Kn   Object_Normal.png"
	* Metalness				"map_ns   Object_Metalness.png"
	* Roughness				"map_Ks   Object_Roughness.png"
	* Ambient occlusion		"map_Ka	  Object_AO.png"
	//Extra
	* Displacement			"map_disp Object_Displace.png

*/

/*
	Types of textures that a material can have
*/
enum class ETextureType
{
	albedo,
	normal,
	metalness,
	roughness,
	ambientOcclusion,
	displacement,
	length
};

class RMaterial : public resource::GResource
{
private:
	/*
		Constantbuffer with constants for the material
		Size: 48 bytes
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
		Size: 32 bytes
		Optimization: save memory with bitwise? send only one number
	*/
	struct properties_t
	{
		int 	hasAlbedo	 = 0;	//4 byte
		int		hasNormal	 = 0;	//4 byte
		int		hasMetalness = 0;	//4 byte
		int		hasRoughness = 0;	//4 byte
		int		hasAoMap	 = 0;	//4 byte
		int		hasDisplace  = 0;	//4 byte
		int		padding[2];			//8 bytes
	};
	properties_t m_properties;
	ComPtr<ID3D11Buffer> m_hasTextureCB;

	/*
		All the textures that material has
	*/
	std::shared_ptr<RTexture> m_textures[UINT(ETextureType::length)];

private:
	//Split the path to only get the filename
	const std::string GetFilename(const std::string& path) const;
	bool CreateConstBuf(const matConstants_t& mat);
	bool CreateConstBuf(const properties_t& mat);

public:
	RMaterial();
	~RMaterial();

	void BindMaterial() const;
	void UnBindMaterial() const;
	void BindDeferredMaterial(ID3D11DeviceContext* context);
	void UnBindDeferredMaterial(ID3D11DeviceContext* context);
	
	//Check if a material has a specific texture
	bool HasTexture(const ETextureType& type) const;

	//Loaded from assimp
	bool Create(aiMaterial* aiMat, const std::string& fileformat);

	// Inherited via GResource
	// Load material with file - mtl files
	virtual bool Create(const std::string& filename) override;

};
