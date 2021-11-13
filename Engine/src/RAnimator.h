#pragma once
#include "AnimStructures.h"
#include "RAnimation.h"

const UINT T2D_BONESLOT = 9;

/*
	Loads in an animator from our custom format ".anim"
	Read Example.anim for more details about how to use it.

	Can be used as an Resource and reused for multiple entities. 
	But will use the same animation in that case.
*/

class RAnimator : public resource::GResource
{
private:
	double							m_currentFrameTime;
	bool							m_useInterpolation;
	std::vector<bone_keyFrames_t>	m_bones;										//Change to only bone_t later

	//AnimationType m_currentAnim;
	//std::unordered_map<AnimationType, std::shared_ptr<RAnimation>> m_animations;

	std::unordered_map<std::string, std::shared_ptr<RAnimation>> m_animations;		//Change to std::unique_ptr<animation_t>

	//Will later be: animation_t* m_defaultAnim;
	std::shared_ptr<RAnimation> m_defaultAnim;
	std::shared_ptr<RAnimation> m_currentAnim;
	std::shared_ptr<RAnimation> m_nextAnim;

	/*
	struct animation_t
	{
		std::shared_ptr<RAnimation> animation;
		double currentFrameTime;
		std::unordermap<std::string, std::array<UINT, 3>> lastKeys;
	}
	*/
	//some kind of double m_currentBlendTime;

	//Matrices that is going up to the GPU - structure buffer
	std::vector<sm::Matrix>			 m_finalMatrices;
	ComPtr<ID3D11Buffer>			 m_bonesSB_Buffer;
	ComPtr<ID3D11ShaderResourceView> m_bonesSB_RSV;

private:
	bool LoadSkeleton(const std::vector<bone_t>& skeleton);
	bool CreateBonesSB();
	void UpdateStructureBuffer();

	AnimationType GetAnimationType(const std::string& name) const;

	//Reset the time of currentFrametime
	void ResetTime();

	//void BlendAnimation();
	//void OneAnimation();

public:
	RAnimator();
	~RAnimator();

	// Inherited via GResource
	// Create from a custom file - something.anim
	virtual bool Create(const std::string& filename) override;

	//Enable or disable interpolation
	void SetInterpolation(bool& toggle);

	//Randomize the starttime of an animation
	void RandomizeTime();

	//Switch animation - not filename
	bool ChangeAnimation(const std::string& name);

	//Update the animation
	void Update();

	//Bind the bones matrices structured buffer
	void Bind();

	//Unbind the bones matrices structured buffer
	void Unbind() const;

};