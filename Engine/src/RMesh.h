#pragma once
#include "VertexStructure.h"
#include "RMaterial.h"

/*
	Load in a single mesh with Assimp 5.0.1

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
	
	struct mesh_t
	{
		ComPtr<ID3D11Buffer> vertexBuffer;
		ComPtr<ID3D11Buffer> indexBuffer;
		UINT			     indexCount = 0;
	};
	
	EMeshType  m_meshType;
	mesh_t	   m_meshInfo;
	RMaterial* m_material;

	//Save the skeleton in a structure: rootbone --> other parts

private:
	bool CreateVertexBuffer(const std::vector<simple_vertex_t>& vertices);
	bool CreateIndexBuffer(const std::vector<UINT>& indices);

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