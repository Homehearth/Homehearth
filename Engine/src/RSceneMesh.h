#pragma once
#include <RMesh.h>

/*
	Loads in scene that consist of alot of meshes with Assimp 5.0.1

*/

class RSceneMesh : public resource::GResource
{
private:
	std::unordered_map<std::string, RMesh*> m_meshes;

public:
	RSceneMesh();
	~RSceneMesh();


};