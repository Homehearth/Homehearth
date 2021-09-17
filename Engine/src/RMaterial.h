#pragma once
#include "RTexture.h"

class RMaterial : public resource::GResource
{
private:
	enum class ETextureType
	{
		diffuse,
		normal,
		metalness,
		roughness,
		length
	};

	std::string m_name		= "";

	sm::Vector3 m_ambient	= {};
	sm::Vector3 m_diffuse	= {};
	sm::Vector3 m_specular	= {};
	float       m_shiniess  = 0;
	float		m_opacity	= 1;  //?
	RTexture*   m_textures[uint8_t(ETextureType::length)] = { nullptr };
	
private:
	sm::Vector3 ConvertAI3D(const aiVector3D& aivec);

public:
	RMaterial();
	~RMaterial();

	void UploadToGPU();

	//Loaded from assimp
	void LoadMaterial(aiMaterial* aiMat);

	// Inherited via GResource
	// Load material with file - mtl files
	virtual bool Create(const std::string& filename) override;

};