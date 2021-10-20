#pragma once
#include "AnimStructures.h"
#include "RAnimation.h"

/*
	Loads in an animator from our custom format ".anim"

	Prefixes:
	* 

*/

class Animator
{
private:
	double		m_frameTime;
	std::string m_currentAnim;	//optimize by ints instead?
	std::string m_nextAnim;

	
	std::vector<bone_t>	m_bones;
	std::unordered_map<std::string, std::shared_ptr<RAnimation>> m_animations;
	//std::array<UINT, 3> lastKeys;

	std::vector<sm::Matrix> m_finalMatrix;
	//Structure that is going up to gpu

	

	//Need to have different vertexshaders for default or skeletal 
	//std::shared_ptr<Shaders::VertexShader> m_vertexShader;	//Get from the resourcemanager

public:
	Animator();
	~Animator();

	//Create from a custom file - something.anim
	//Create()

	//Set what bones to use from an model
	void SetBones(const std::vector<bone_t>& bones);


	void Update();

	//Bind()

	//Unbind()

};