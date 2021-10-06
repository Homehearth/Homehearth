#pragma once
#include "CommonStructures.h"
#include "RMaterial.h"
struct aiMesh;

/*
	Load in a model/scene of multiple meshes with Assimp 5.0.1
	Format supported:
	* FBX
	* OBJ

	Supports files with multiple submeshes and multiple materials
	Combines all the submeshes with same material to one.
	This is to optimized and lower the amount of drawcalls.

	TODO:
	* Fix loading in bones
*/

class RModel : public resource::GResource
{
private:
	struct mesh_t
	{
		ComPtr<ID3D11Buffer> vertexBuffer;
		ComPtr<ID3D11Buffer> indexBuffer;
		UINT				 indexCount = 0;
	};
	
	std::vector<mesh_t>						m_meshes;
	std::vector<std::shared_ptr<RMaterial>> m_materials;

	//dx::BoundingBox							m_globalCollider;	//total boundingbox
	std::vector<dx::BoundingBox>			m_colliders;	//accurate

	//Save the skeleton in a structure: rootbone --> other parts

private:
	//Get the end of file. Searches for "."
	const std::string GetFileFormat(const std::string& filename) const;
	/*
		Combines multiple submeshes that uses the same material to one.
		This is to avoid to many drawcalls per RModel
	*/
	bool CombineMeshes(std::vector<aiMesh*>& submeshes);

	//Creating buffers
	bool CreateVertexBuffer(const std::vector<simple_vertex_t>& vertices, mesh_t& mesh);
	bool CreateIndexBuffer(const std::vector<UINT>& indices, mesh_t& mesh);

public:
	RModel();
	~RModel();

	//Change the material to something else
	bool ChangeMaterial(const std::string& mtlfile);

	/*
		Render all of the submeshes in the RModel with correct material
	*/
	void Render() const;

	// Inherited via GResource
	virtual bool Create(const std::string& filename) override;

};