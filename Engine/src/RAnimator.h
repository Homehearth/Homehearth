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
	bool				m_useInterpolation;
	std::vector<bone_t>	m_bones;

	//States
	EAnimationType m_currentType;
	EAnimationType m_nextType;
	EAnimationType m_defaultType;
	
	struct animation_t
	{
		std::shared_ptr<RAnimation>					animation;
		double										frameTimer	= 0;
		double										blendTimer	= 0;
		double										lastTick	= 0;
		std::unordered_map<std::string, lastKeys_t> lastKeys;
		bool										reachedEnd	= false;
	};

	//All the animations
	std::unordered_map<EAnimationType, animation_t> m_animations;

	//Blendstates
	std::unordered_map<blendstate_t, double, blend_hash_fn> m_blendStates;

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
	EAnimationType StringToAnimationType(const std::string& name) const;

	//Reset the time of currentFrametime
	void ResetAnimation(const EAnimationType& type);

	//Update the time for an animation
	void UpdateTime(const EAnimationType& type);

	void RegularAnimation();
	void BlendAnimations();
	void SwapAnimationState();

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

	//Queue up what animation to play next
	bool ChangeAnimation(const EAnimationType& type);

	//Get the enum of what state the animator is in
	const EAnimationType& GetCurrentState() const;

	//Update the animation
	void Update();

	//Bind the bones matrices structured buffer
	void Bind();

	//Unbind the bones matrices structured buffer
	void Unbind() const;

};