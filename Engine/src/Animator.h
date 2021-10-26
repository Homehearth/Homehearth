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
	std::unordered_map<std::string, std::shared_ptr<RAnimation>> m_animations;


	//Structure that is going up to gpu
	//ComPtr<ID3D11Buffer> m_nrOfBonesBuffer;

	std::vector<sm::Matrix>			 m_finalMatrices;
	ComPtr<ID3D11Buffer>			 m_bonesSB_Buffer;
	ComPtr<ID3D11ShaderResourceView> m_bonesSB_RSV;		//structurebuffer

private:
	bool LoadModel(const std::string& filename);
	void LoadAnimations(const std::vector<std::string>& animNames);

	bool CreateBonesSB();

	//bool UpdateNrOfBones();
	void UpdateStructureBuffer();
	void Bind() const;
	void Unbind() const;

public:
	Animator();
	~Animator();

	//Create from a custom file - something.anim
	bool Create(const std::string& filename);

	//Update the animation
	void Update();

	//Render the current pose
	void Render();

};