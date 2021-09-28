#pragma once
#include "RTexture.h"

struct aiMaterial;

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

	std::string m_name;
	sm::Vector3 m_ambient;
	sm::Vector3 m_diffuse;
	sm::Vector3 m_specular;
	float       m_shiniess;
	float		m_opacity;
	RTexture*   m_textures[UINT(ETextureType::length)];

private:
	//Split the path to only get the filename
	std::string GetFilename(const std::string& path);

public:
	RMaterial();
	~RMaterial();

	//[TODO] Fix later
	void UploadToGPU();

	//Loaded from assimp
	void LoadMaterial(aiMaterial* aiMat);

	// Inherited via GResource
	// Load material with file - mtl files
	virtual bool Create(const std::string& filename) override;

};
