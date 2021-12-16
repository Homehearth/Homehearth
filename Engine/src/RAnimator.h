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

	EAnimationType							m_currentState;		//The main focus animation
	EAnimationType							m_blendState;		//The animation we blending to
	EAnimationType							m_partialState;		//Secondary animation that can be played on a part of the body
	std::queue<EAnimationType>				m_queue;			//Queue of animations to play in order
	bool									m_blendDir;			//True: toward blend, False: toward current

	struct animation_t
	{
		//Shared data
		std::shared_ptr<RAnimation>					animation;
		
		//Specific data for this animation in this animator
		float										frameTimer			= 0;
		float										lastTick			= 0;
		std::unordered_map<std::string, lastKeys_t> lastKeys;
		std::string									partialStartBone	= "";	//Start bone of partial animation
		std::string									partialEndBone		= "";	//End bone of partial animation - okay to leave blank
		bool										stayAtEnd			= false;
	};

	std::unordered_map<EAnimationType, animation_t>						m_animations;
	std::unordered_map<animstate_t, animstateInfo, animstate_hash_fn>	m_states;

	//Matrices that is going up to the GPU - structure buffer - in modelspace
	std::vector<sm::Matrix>			 m_localMatrices;
	ComPtr<ID3D11Buffer>			 m_bonesSB_Buffer;
	ComPtr<ID3D11ShaderResourceView> m_bonesSB_RSV;

private:
	bool LoadSkeleton(const std::vector<bone_t>& skeleton);
	bool CreateBonesSB();
	void UpdateStructureBuffer();

	//Convert a string to enum
	EAnimationType StringToAnimationType(const std::string& name) const;

	//Reset the time and last bones
	void ResetAnimation(const EAnimationType& type);

	//Update the time for an animation. Return false when reached end
	bool UpdateTime(const EAnimationType& type);
	//Update the blend timer for a transition between two animations. Return true on success. 
	//Lerptime will be returned as a parameter on success.
	bool UpdateBlendTime(const EAnimationType& from, const EAnimationType& to, float& lerpTime);

	//Different types of doing animations
	void StandardAnim();			//Current
	void BlendTwoAnims();			//Current + blend
	void PartialAnim();				//Current + partial
	void BlendPartialAnim();		//Current + blend + partial

	EAnimStatus GetAnimStatus() const;
	bool ReadyToBlend(const EAnimationType& from, const EAnimationType& to) const;
	bool HasStayAtEndAnim();

	//Check what animations that is queued up
	void CheckQueue();

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
	void ChangeAnimation(const EAnimationType& type);

	//Get the enum of what state the animator is in
	const EAnimationType& GetCurrentState() const;

	//Get a the local matrix of a bone
	const sm::Matrix GetLocalMatrix(const std::string& bonename) const;

	//Update the animation
	void Update();

	//Bind/unbind the bones matrices structured buffer
	void Bind();
	void Unbind() const;
};
