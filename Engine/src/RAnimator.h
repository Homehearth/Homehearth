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
	double							m_lastTick;
	bool							m_useInterpolation;
	std::vector<bone_keyFrames_t>	m_bones;				//Change to only bone_t later

	//States
	AnimationType m_currentType;
	AnimationType m_nextType;
	AnimationType m_defaultType;

	struct animation_t
	{
		std::shared_ptr<RAnimation> animation;
		double						frameTimer = 0;
		double						blendTimer = 0;
	};

	//All the animations
	std::unordered_map<AnimationType, animation_t> m_animations;

	//Blendstates
	/*struct twoStates
	{
		AnimationType from;
		AnimationType to;
	};
	std::unordered_map<twoStates, double> blendStates;*/

	//Matrices that is going up to the GPU - structure buffer
	std::vector<sm::Matrix>			 m_finalMatrices;
	ComPtr<ID3D11Buffer>			 m_bonesSB_Buffer;
	ComPtr<ID3D11ShaderResourceView> m_bonesSB_RSV;

private:
	//All the bones
	bool LoadSkeleton(const std::vector<bone_t>& skeleton);
	bool CreateBonesSB();
	void UpdateStructureBuffer();

	//Enum
	AnimationType StringToAnimationType(const std::string& name) const;

	//Reset the time of currentFrametime
	void ResetLastKeys();
	void ResetAnimation(const AnimationType& type);

	//Return if we shall update
	bool UpdateTime();

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
	bool ChangeAnimation(const AnimationType& type);

	//Get the enum of what state the animator is in
	const AnimationType& GetCurrentState() const;

	//Update the animation
	void Update();

	//Bind the bones matrices structured buffer
	void Bind();

	//Unbind the bones matrices structured buffer
	void Unbind() const;

};