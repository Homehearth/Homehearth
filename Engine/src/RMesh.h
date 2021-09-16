#pragma once
#include "VertexStructure.h"
#include "RTexture.h"

/*
	Loads in mesh information with Assimp 5.0.1
	Can load in submeshes and multiple materials

	TODO:
	* Fix loading in bones
*/

class RMesh : public resource::GResource
{
private:
	enum class EMeshType
	{
		staticMesh,
		skeletalMesh	//Has bones
	};
	enum class ETextureType
	{
		diffuse,
		normal,
		metalness,
		roughness,
		length
	};

	struct material_t
	{
		sm::Vector3 ambient									= {};
		sm::Vector3 diffuse									= {};
		sm::Vector3 specular								= {};
		float       shiniess								= 0.0f;
		float		opacity								    = 1.0f;  //?
		RTexture*   textures[uint8_t(ETextureType::length)] = { nullptr };
	};

	/*
		Each mesh has its own buffers and materials
	*/
	struct mesh_t
	{
		ComPtr<ID3D11Buffer> vertexBuffer;
		ComPtr<ID3D11Buffer> indexBuffer;
		uint32_t			 indexCount = 0;
		uint16_t			 materialID = 0;
	};

	EMeshType m_meshType;

	//All the materials and meshes
	std::vector<material_t> m_materials;
	std::vector<mesh_t>		m_meshes;

	//Save the skeleton in a structure: rootbone --> other parts

private:
	bool CreateVertexBuffer(const simple_vertex_t* data, const size_t& size, mesh_t& mesh);
	bool CreateIndexBuffer(const size_t* data, const size_t& size, mesh_t& mesh);
	void AddTextures(material_t& mat, const aiMaterial* aiMat);

public:
	RMesh();
	~RMesh();

	/*
		Render the mesh depending on type
		TODO: Skeletal special
	*/
	void Render();

	// Inherited via GResource
	virtual bool Create(const std::string& filename) override;

};