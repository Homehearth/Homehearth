#pragma once
#include "CommonStructures.h"
#include "RMaterial.h"
struct aiMesh;
struct aiScene;
struct aiNode;

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
	/*
		Default data for model 
	*/
	struct submesh_t
	{
		ComPtr<ID3D11Buffer>			vertexBuffer;
		ComPtr<ID3D11Buffer>			indexBuffer;
		UINT							indexCount = 0;
		std::shared_ptr<RMaterial>		material;
	};
	std::vector<submesh_t>				m_meshes;
	std::vector<light_t>				m_lights;
	
	/*
		Model that has skeleton
	*/
	bool								m_hasSkeleton;
	struct bone_t
	{
		std::string name		= "";
		sm::Matrix	inverseBind = {};
		UINT		parentIndex = 0;
	};
	std::vector<bone_t>					m_allBones;

	/*
	Read from bones when doing update
	for (i = 0; i < bones.size(); i++)
		if (parent == -1)
			modelMatrix[i] = local;
		else
			modelMatrix[i] = modelMatrix[parent] * local;
		
		finalMatrix[i] = modelMatrix[i] * bones[i].inverse;
	*/

	//Need to have different vertexshaders for default or skeletal 
	//std::shared_ptr<Shaders::VertexShader> m_vertexShader;	//Get from the resourcemanager

private:
	//Get the end of file. Searches for "."
	const std::string GetFileFormat(const std::string& filename) const;
	
	/*
		Combines multiple submeshes that uses the same material to one.
		This is to avoid to many drawcalls per RModel.
	*/
	bool CombineMeshes(std::vector<aiMesh*>& submeshes, const aiNode* root, submesh_t& submesh);

	//Creating buffers
	bool CreateVertexBuffer(const std::vector<anim_vertex_t>&	vertices, submesh_t& mesh);
	bool CreateVertexBuffer(const std::vector<simple_vertex_t>& vertices, submesh_t& mesh);
	bool CreateIndexBuffer(const std::vector<UINT>& indices, submesh_t& mesh);

	//Loading data from assimp
	void LoadLights(const aiScene* scene);
	void LoadMaterial(const aiScene* scene, const UINT& matIndex, bool& useMTL, submesh_t& inoutMesh) const;

public:
	RModel();
	~RModel();

	//Get function to check if the model has loaded in any bones
	bool HasSkeleton() const;

	//Get the vector of lights
	const std::vector<light_t>& GetLights() const;

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
	void RenderDeferred(ID3D11DeviceContext* context);

	// Inherited via GResource
	virtual bool Create(const std::string& filename) override;

};