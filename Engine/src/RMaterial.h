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

	std::string m_name;
	sm::Vector3 m_ambient;
	sm::Vector3 m_diffuse;
	sm::Vector3 m_specular;
	float       m_shiniess;
	float		m_opacity;
	RTexture*   m_textures[UINT(ETextureType::length)];
	
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