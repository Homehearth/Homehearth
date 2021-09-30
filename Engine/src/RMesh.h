#pragma once
#include "CommonStructures.h"
#include "RMaterial.h"

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
	EMeshType					m_meshType;
	ComPtr<ID3D11Buffer>		m_vertexBuffer;
	ComPtr<ID3D11Buffer>		m_indexBuffer;
	UINT						m_indexCount;
	std::shared_ptr<RMaterial>	m_material;

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