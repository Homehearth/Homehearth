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

	The model has a BoundingSphere that covers everything.
	There is also multiple boundingboxes, one per submesh

	TODO:
	* Fix loading in bones
*/

class RModel : public resource::GResource
{
private:
	//Each submesh has some data
	struct submesh_t
	{
		ComPtr<ID3D11Buffer>			vertexBuffer;
		ComPtr<ID3D11Buffer>			indexBuffer;
		UINT							indexCount = 0;
		std::shared_ptr<RMaterial>		material;
	};
	std::vector<submesh_t>			m_meshes;
	
	//Colliders. Sphere covers the mesh
	dx::BoundingSphere				m_boundingSphere;
	std::vector<dx::BoundingBox>	m_boundingBoxes;

	//Save the skeleton in a structure: rootbone --> other parts

private:
	//Get the end of file. Searches for "."
	const std::string GetFileFormat(const std::string& filename) const;
	
	/*
		Add a boundingbox to the submesh. A submesh can have multiple boundingboxes.
		This is to avoid losing precision
	*/
	void AddBoundingBox(const aiMesh*& aimesh, submesh_t& submesh);

	/*
		Create a global sphere
	*/
	void CalcBoundingSphere();

	/*
		Combines multiple submeshes that uses the same material to one.
		This is to avoid to many drawcalls per RModel.
	*/
	bool CombineMeshes(std::vector<aiMesh*>& submeshes, submesh_t& submesh);

	//Creating buffers
	bool CreateVertexBuffer(const std::vector<simple_vertex_t>& vertices, submesh_t& mesh);
	bool CreateIndexBuffer(const std::vector<UINT>& indices, submesh_t& mesh);

public:
	RModel();
	~RModel();

	//Change the material to something else
	bool ChangeMaterial(const std::string& mtlfile);

	//Get colliders
	const dx::BoundingSphere& GetBoundingSphere() const;
	const std::vector<dx::BoundingBox>& GetBoundingBoxes() const;

	/*
		Render all of the submeshes in the RModel with correct material
	*/
	void Render() const;

	// Inherited via GResource
	virtual bool Create(const std::string& filename) override;

};