#pragma once
#include "AnimStructures.h"
#include "RModel.h"
#include "RAnimation.h"

const UINT T2D_BONESLOT = 11;

/*
	Loads in an animator from our custom format ".anim"

	Prefixes:
	* 

*/

class Animator
{
private:
	double					m_frameTime;
	std::string				m_currentAnim;	//optimize by ints instead?
	std::string				m_nextAnim;

	std::shared_ptr<RModel>			m_model;
	std::vector<bone_keyFrames_t>	m_bones;
	std::vector<sm::Matrix>			m_finalMatrix;
	std::unordered_map<std::string, std::shared_ptr<RAnimation>> m_animations;


	//Structure that is going up to gpu

private:
	bool LoadModel(const std::string& filename);
	void LoadAnimations(const std::vector<std::string>& animNames);

public:
	Animator();
	~Animator();

	//Create from a custom file - something.anim
	bool Create(const std::string& filename);

	void Render();

};