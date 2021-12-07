#pragma once
#include "CommonStructures.h"
#include "AnimStructures.h"
#include "RMaterial.h"
#include "Mesh.h"

//Combines meshes with the same material to one
const bool OPTIMIZE_MODEL = true;

//Define structs to avoid 
struct aiMesh;
struct aiScene;
struct aiNode;

/*
	Load in a model/scene of multiple meshes with Assimp 5.0.1
	
	*  Formats supported: FBX and OBJ

	*  Supports files with multiple submeshes and multiple materials.
	   Combines all the submeshes with same material to one.
	   This is to optimized and lower the amount of drawcalls.

	*  The model can change material to any other mtl-file.
	   Order in the mtl-file is important for best result.

	*  Supports loading in lights from the FBX-format.

	*  Loads a skeleton, which is a hierchi of bones/joints
	   that can affect the model.
*/

class RModel : public resource::GResource
{
private:
	std::string						m_resourcename;
	std::vector<Mesh>				m_meshes;
	std::vector<light_t>			m_lights;
	std::vector<bone_t>				m_allBones;

private:	
	/*
		Creates one mesh with one material.
		Will cause many drawcalls per RModel.
	*/
	bool CreateOneMesh(aiMaterial* aimat, aiMesh* aimesh, const std::unordered_map<std::string, UINT>& boneMap);
	
	/*
		Combines multiple submeshes that uses the same material to one.
		This is to avoid to many drawcalls per RModel.
	*/
	bool CombineMeshes(aiMaterial* aimat, std::vector<aiMesh*>& aimeshes, const std::unordered_map<std::string, UINT>& boneMap);

	//Loading data from assimp
	void LoadLights(const aiScene* scene);

	//Bone structure
	void BoneHierchy(aiNode* node, std::unordered_map<std::string, bone_t>& nameToBone);

public:
	RModel();
	~RModel();

	//Get function to check if the model has loaded in any bones
	bool HasSkeleton() const;

	//Get all the bones for the model - Most useful for the animator
	const std::vector<bone_t>& GetSkeleton() const;

	//Get the vector of lights
	const std::vector<light_t>& GetLights() const;

	//Get all the meshes that the model is holding
	const std::vector<Mesh>& GetMeshes() const;

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
	void Render(ID3D11DeviceContext* context);

	// Inherited via GResource
	virtual bool Create(const std::string& filename) override;
};
