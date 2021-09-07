#pragma once
#include "MeshStructure.h"
#include "Buffers.h"
#include "ResourceManager.h"

/*
	Loads in mesh information with Assimp 5.0.1
	Can load in submeshes and multiple materials


	Still a bit of a sketchy version :,(


	TODO:
	* Convert it to ECS
	* Fix loading in bones
	* Rendering the mesh on screen
	
*/

enum class EMeshType
{
	staticMesh,		//Will not change
	skeletalMesh	//Can be used for animations
};

struct submesh_t
{
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	size_t				 indexCount  = 0;
	unsigned int		 materialID  = 0;
};

/*
	Struct for bones?
*/


class Mesh
{
private:
	EMeshType m_meshType;
	bool m_isVisible;

	//All the materials and meshes
	std::vector<material_t> m_materials;
	std::vector<submesh_t>  m_meshes;

	/*
		Save the skeleton in a structure
		rootbone --> other parts
	*/

private:
	void AddTextures(material_t& mat, const aiMaterial* aiMat);

public:
	Mesh();
	~Mesh();

	/*
		Using assimp to import the mesh
	*/
	bool Initialize(const std::string& filepath);

	/*
		Render the mesh depending on type
	*/
	void Render();

};