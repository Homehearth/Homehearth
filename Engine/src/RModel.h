#pragma once
#include "CommonStructures.h"
#include "RMaterial.h"
struct aiMesh;

/*
	Load in a model/scene of multiple meshes with Assimp 5.0.1
	Formats supported:
	* FBX
	* OBJ

	Supports files with multiple submeshes and multiple materials
	Combines all the submeshes with same material to one.
	This is to optimized and lower the amount of drawcalls.

	The model can change material to any other mtl-file.
	Order in the mtl-file is important.

	Can load in lights from the FBX-format.

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
	std::vector<submesh_t>				m_meshes;
	std::vector<light_t>				m_lights;
	
	//Save the skeleton in a structure: rootbone --> other parts

private:
	//Get the end of file. Searches for "."
	const std::string GetFileFormat(const std::string& filename) const;
	
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

	//Get the vector of lights
	const std::vector<light_t>& GetLights() const { return m_lights; }

	/*
		Change the material to other. Uses a mtlfile.
		Limited to how many materials was set from start.
		Example: RModel has 3 materials from start.
		* mtl-file has 5 materials only 3 will be applied.
		* mtl-file only has 1 material, the reset will be reseted to none
	*/
	bool ChangeMaterial(const std::string& mtlfile);

	/*
		Render all of the submeshes in the RModel with correct material
	*/
	void Render() const;

	// Inherited via GResource
	virtual bool Create(const std::string& filename) override;

};