#pragma once
#include "RTexture.h"

class RMaterial : public resource::GResource
{
private:
	enum class ETextureType
	{
		albedo,
		normal,
		metalness,
		roughness,
		aoMap,	//Ambient occulution
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
	bool LoadMaterial(aiMaterial* aiMat);

	// Inherited via GResource
	// Load material with file - mtl files
	virtual bool Create(const std::string& filename) override;

};