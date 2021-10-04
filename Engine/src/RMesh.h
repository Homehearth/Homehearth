#pragma once
#include "CommonStructures.h"
#include "RMaterial.h"
//struct RMaterial;
struct aiMesh;

/*
	Load in a model (".obj", ".fbx") with Assimp 5.0.1

	Can load in a mesh with submeshes. (combines all to one in create)
	Limited to only one material per RMesh at this time.
	More materials == lower performance
	This is because we have to change material between every drawcall

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
	EMeshType m_meshType;

	struct mesh_t
	{
		ComPtr<ID3D11Buffer> vertexBuffer;
		ComPtr<ID3D11Buffer> indexBuffer;
		UINT				 indexCount = 0;
	};
	
	std::vector<mesh_t>						m_meshes;
	std::vector<std::shared_ptr<RMaterial>> m_materials;

	//Save the skeleton in a structure: rootbone --> other parts

private:
	//Get the end of file. Searches for "."
	const std::string GetFileFormat(const std::string& filename);
	/*
		Combines multiple submeshes that uses the same material to one.
		This is to avoid to many drawcalls per RMesh
	*/
	bool CombineMeshes(std::vector<aiMesh*>& submeshes);

	//Creating buffers
	bool CreateVertexBuffer(const std::vector<simple_vertex_t>& vertices, mesh_t& mesh);
	bool CreateIndexBuffer(const std::vector<UINT>& indices, mesh_t& mesh);

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