#pragma once
#include "AnimStructures.h"
#include "RAnimation.h"

const UINT T2D_BONESLOT = 9;

/*
	Loads in an animator from our custom format ".anim"
	
	Prefixes:
	* Load in skeleton-hierchy:		"skel			Knight.fbx"
	* Load in an animation:			"anim			idle		idle.fbx"
	* Set current start animation:  "currentAnim	idle"

	Can be used as an Resource and reused for multiple entities. 
	But will use the same animation in that case.
	
*/

class RAnimator : public resource::GResource
{
private:
	double							m_currentFrameTime;
	bool							m_useInterpolation;
	std::vector<bone_keyFrames_t>	m_bones;
	std::unordered_map<std::string, std::shared_ptr<RAnimation>> m_animations;

	std::shared_ptr<RAnimation> m_currentAnim;
	//std::shared_ptr<RAnimation> m_nextAnim;

	//Matrices that is going up to the GPU - structure buffer
	std::vector<sm::Matrix>			 m_finalMatrices;
	ComPtr<ID3D11Buffer>			 m_bonesSB_Buffer;
	ComPtr<ID3D11ShaderResourceView> m_bonesSB_RSV;

private:
	bool LoadSkeleton(const std::vector<bone_t>& skeleton);
	bool CreateBonesSB();
	void UpdateStructureBuffer();

public:
	RAnimator();
	~RAnimator();

	//Enable or disable interpolation
	void SetInterpolation(bool& toggle);

	// Inherited via GResource
	// Create from a custom file - something.anim
	virtual bool Create(const std::string& filename) override;

	//Update the animation
	void Update();

	//Bind the bones matrices structured buffer
	void Bind();

	//Unbind the bones matrices structured buffer
	void Unbind() const;

};