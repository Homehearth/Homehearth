#pragma once
#include "RTexture.h"

/*
	TODO:
	* Load in mlt-files
*/

class RMaterial : public resource::GResource
{
private:
	enum class ETextureType
	{
		diffuse,	//Albedo
		normal,
		metalness,
		roughness,
		length
	};
	//Add ambient occlution map

	std::string m_name;
	sm::Vector3 m_ambient;
	sm::Vector3 m_diffuse;
	sm::Vector3 m_specular;
	float       m_shiniess;
	float		m_opacity;
	RTexture*   m_textures[UINT(ETextureType::length)];
	
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