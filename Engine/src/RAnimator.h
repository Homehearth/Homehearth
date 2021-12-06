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
	bool									m_useInterpolation;
	std::vector<bone_t>						m_bones;
	std::unordered_map<std::string, UINT>	m_nameToBone;

	std::queue<EAnimationType> m_queue;
	//std::vector<EAnimationType> m_queue;

	EAnimationType m_currentState;
	EAnimationType m_nextState;
	EAnimationType m_upperState;

	
	struct animation_t
	{
		//Shared data
		std::shared_ptr<RAnimation>					animation;

		//Specific data for this animation in this animator
		double										frameTimer	= 0;
		double										blendTimer	= 0;
		double										currentTick	= 0;
		std::unordered_map<std::string, lastKeys_t> lastKeys;
		bool										reachedEnd	= false;

		//Will not get reseted
		bool										isUpperBody	= false;
		std::string									devideBone  = "";
	};

	//All the animations
	std::unordered_map<EAnimationType, animation_t>			m_animations;
	std::unordered_map<blendstate_t, double, blend_hash_fn> m_blendStates;

	//Matrices that is going up to the GPU - structure buffer - in modelspace
	std::vector<sm::Matrix>			 m_localMatrices;
	ComPtr<ID3D11Buffer>			 m_bonesSB_Buffer;
	ComPtr<ID3D11ShaderResourceView> m_bonesSB_RSV;

private:
	//All the bones
	bool LoadSkeleton(const std::vector<bone_t>& skeleton);
	bool CreateBonesSB();
	void UpdateStructureBuffer();

	//Convert a string to enum
	EAnimationType StringToAnimationType(const std::string& name) const;

	//Reset the time of currentFrametime
	void ResetAnimation(const EAnimationType& type);

	//Update the time for an animation. Return false when reached end
	bool UpdateTime(const EAnimationType& type);

	//Animation with only one state
	void RegularAnimation(const EAnimationType& state);

	//Blend between two animations
	void BlendAnimations(const EAnimationType& state1, const EAnimationType& state2);

	//Animate the lower body as usual and swap to upper when reached a bone
	void UpperLowerbodyAnimation(const EAnimationType& upper, const EAnimationType& lower);

	void BlendUpperBodyAnimations(const EAnimationType& state1, const EAnimationType& state2, const EAnimationType& upper);

	void SwapAnimationState();

	/*
		Upperbody update

		Fix a m_bonePoseAbsolute - can update from other parts later

		BlendAnimations() from start until we reached the devidebone
		BlendAnimations(upper, ???)


		for (from devideBone -> till end)
		{
			blend if needed between this and other
		}
	*/

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

	//Get a the local matrix of a bone
	const sm::Matrix GetLocalMatrix(const std::string& bonename) const;

	//Update the animation
	void Update();

	//Bind the bones matrices structured buffer
	void Bind();

	//Unbind the bones matrices structured buffer
	void Unbind() const;

};