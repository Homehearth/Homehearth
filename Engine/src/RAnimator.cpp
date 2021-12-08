#include "EnginePCH.h"
#include "RAnimator.h"

RAnimator::RAnimator()
{
	m_useInterpolation	= true;
	m_currentState	= EAnimationType::NONE;
	m_nextState		= EAnimationType::NONE;
	m_upperState	= EAnimationType::NONE;

}

RAnimator::~RAnimator()
{
	for (auto& anim : m_animations)
	{
		anim.second.lastKeys.clear();
	}
	m_animations.clear();
	m_blendStates.clear();
	m_bones.clear();
	m_localMatrices.clear();
	m_animations.clear();
	m_nameToBone.clear();
}

bool RAnimator::LoadSkeleton(const std::vector<bone_t>& skeleton)
{
	//Clear previous bones and matrices if needed
	m_localMatrices.clear();
	m_bones.clear();

	bool loaded = false;
	if (!skeleton.empty())
	{
		m_bones = skeleton;
		m_localMatrices.resize(m_bones.size(), sm::Matrix::Identity);

		for (size_t i = 0; i < m_bones.size(); i++)
		{
			m_nameToBone[m_bones[i].name] = static_cast<UINT>(i);
		}

		//Creating the buffers needed for the GPU
		if (CreateBonesSB())
			loaded = true;
	}
	return loaded;
}

void RAnimator::SetInterpolation(bool& toggle)
{
	m_useInterpolation = toggle;
}

void RAnimator::RandomizeTime()
{
	auto iterator = m_animations.find(m_currentState);

	//Check that it is okay to edit this animation
	if (iterator != m_animations.end())
	{
		//Randomizes a double between 0 - duration
		iterator->second.frameTimer = std::rand() / m_animations[m_currentState].animation->GetDuration();
	}
}

bool RAnimator::CreateBonesSB()
{
	UINT nrOfMatrices = static_cast<UINT>(m_localMatrices.size());

	//Can't create a buffer if there is no bones...
	if (m_localMatrices.empty())
	{
		return false;
	}

	/*
		Buffer
	*/
	D3D11_BUFFER_DESC desc		= {};
	desc.Usage					= D3D11_USAGE_DYNAMIC;
	desc.ByteWidth				= sizeof(sm::Matrix) * nrOfMatrices;
	desc.BindFlags				= D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags			= D3D11_CPU_ACCESS_WRITE;
	desc.StructureByteStride	= sizeof(sm::Matrix);
	desc.MiscFlags				= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&desc, nullptr, m_bonesSB_Buffer.GetAddressOf());
	if (FAILED(hr))
		return false;
	
	/*
		Resource view
	*/
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format							= DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension					= D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.BufferEx.FirstElement			= 0;
	srvDesc.BufferEx.Flags					= 0;
	srvDesc.BufferEx.NumElements			= nrOfMatrices;

	hr = D3D11Core::Get().Device()->CreateShaderResourceView(m_bonesSB_Buffer.Get(), &srvDesc, m_bonesSB_RSV.GetAddressOf());

	return !FAILED(hr);
}

void RAnimator::UpdateStructureBuffer()
{
	D3D11_MAPPED_SUBRESOURCE submap;
	D3D11Core::Get().DeviceContext()->Map(m_bonesSB_Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &submap);
	memcpy(submap.pData, &m_localMatrices[0], sizeof(sm::Matrix) * m_localMatrices.size());
	D3D11Core::Get().DeviceContext()->Unmap(m_bonesSB_Buffer.Get(), 0);
}

EAnimationType RAnimator::StringToAnimationType(const std::string& name) const
{
	EAnimationType type = EAnimationType::NONE;
	
	std::unordered_map<std::string, EAnimationType> animationMap
	{
		{"NONE",				EAnimationType::NONE},
		{"IDLE",				EAnimationType::IDLE},
		{"MOVE",				EAnimationType::MOVE},
		{"PRIMARY_ATTACK",		EAnimationType::PRIMARY_ATTACK},
		{"SECONDARY_ATTACK",	EAnimationType::SECONDARY_ATTACK},
		{"ABILITY1",			EAnimationType::ABILITY1},
		{"ABILITY2",			EAnimationType::ABILITY2},
		{"ABILITY3",			EAnimationType::ABILITY3},
		{"ABILITY4",			EAnimationType::ABILITY4},
		{"TAKE_DAMAGE",			EAnimationType::TAKE_DAMAGE},
		{"PLACE_DEFENCE",		EAnimationType::PLACE_DEFENCE},
		{"DEAD",				EAnimationType::DEAD}
	};

	//Search for the keyword
	if (animationMap.find(name) != animationMap.end())
	{
		type = animationMap[name];
	}
	return  type;
}

void RAnimator::ResetAnimation(const EAnimationType& type)
{
	if (m_animations.find(type) != m_animations.end())
	{
		animation_t* anim	= &m_animations[type];
		anim->frameTimer	= 0;
		anim->blendTimer	= 0;
		anim->reachedEnd	= false;
		anim->currentTick	= 0;

		for (auto& key : anim->lastKeys)
		{
			key.second = { 0,0,0 };
		}
	}
}

bool RAnimator::UpdateTime(const EAnimationType& type)
{
	bool updateSuccess = true;
	animation_t* anim = &m_animations[type];
	if (anim)
	{
		//We don't add to the timer if we reached the end
		if (!anim->reachedEnd)
		{
			double tick = anim->animation->GetTicksPerFrame() * Stats::Get().GetFrameTime();

			if (anim->currentTick != tick)
			{
				anim->currentTick = tick;
				anim->frameTimer += tick;

				//Reached end
				if (anim->frameTimer >= anim->animation->GetDuration())
				{
					if (anim->animation->IsLoopable())
					{
						anim->frameTimer = fmod(anim->frameTimer, anim->animation->GetDuration());
					}
					else
					{
						anim->reachedEnd = true;
						updateSuccess = false;
					}
				}
			}
		}
		else
		{
			updateSuccess = false;
		}
	}	
	return updateSuccess;
}

void RAnimator::RegularAnimation(const EAnimationType& state)
{
	animation_t* anim = &m_animations[state];

	if (anim)
	{
		std::vector<sm::Matrix> bonePoseAbsolute;
		bonePoseAbsolute.resize(m_bones.size(), sm::Matrix::Identity);

		//Go through all the bones
		for (size_t i = 0; i < m_bones.size(); i++)
		{
			sm::Matrix bonePoseRelative = anim->animation->GetMatrix(m_bones[i].name, anim->frameTimer, anim->lastKeys[m_bones[i].name].keys, m_useInterpolation);

			if (m_bones[i].parentIndex == -1)
				bonePoseAbsolute[i] = bonePoseRelative;
			else
				bonePoseAbsolute[i] = bonePoseRelative * bonePoseAbsolute[m_bones[i].parentIndex];

			m_localMatrices[i] = m_bones[i].inverseBind * bonePoseAbsolute[i];
		}
		bonePoseAbsolute.clear();
	}
}

void RAnimator::BlendAnimations(const EAnimationType& state1, const EAnimationType& state2)
{
	animation_t* anim1 = &m_animations[state1];
	animation_t* anim2 = &m_animations[state2];

	if (anim1 && anim2)
	{
		std::vector<sm::Matrix> bonePoseAbsolute;
		bonePoseAbsolute.resize(m_bones.size(), sm::Matrix::Identity);

		for (size_t i = 0; i < m_bones.size(); i++)
		{
			//From 
			sm::Vector3 pos1	= anim1->animation->GetPosition(m_bones[i].name, anim1->frameTimer, anim1->lastKeys[m_bones[i].name].keys[0], m_useInterpolation);
			sm::Vector3 scl1	= anim1->animation->GetScale(	m_bones[i].name, anim1->frameTimer, anim1->lastKeys[m_bones[i].name].keys[1], m_useInterpolation);
			sm::Quaternion rot1 = anim1->animation->GetRotation(m_bones[i].name, anim1->frameTimer, anim1->lastKeys[m_bones[i].name].keys[2], m_useInterpolation);
			//To
			sm::Vector3 pos2	= anim2->animation->GetPosition(m_bones[i].name, anim2->frameTimer, anim2->lastKeys[m_bones[i].name].keys[0], m_useInterpolation);
			sm::Vector3 scl2	= anim2->animation->GetScale(	m_bones[i].name, anim2->frameTimer, anim2->lastKeys[m_bones[i].name].keys[1], m_useInterpolation);
			sm::Quaternion rot2 = anim2->animation->GetRotation(m_bones[i].name, anim2->frameTimer, anim2->lastKeys[m_bones[i].name].keys[2], m_useInterpolation);

			float lerpTime			= float(m_animations[state1].blendTimer / m_blendStates.at({state1, state2}));
			sm::Vector3 lerpPos		= sm::Vector3::Lerp(pos1, pos2, lerpTime);
			sm::Vector3 lerpScl		= sm::Vector3::Lerp(scl1, scl2, lerpTime);
			sm::Quaternion lerpRot	= sm::Quaternion::Slerp(rot1, rot2, lerpTime);
			lerpRot.Normalize();

			sm::Matrix bonePoseRelative = sm::Matrix::CreateScale(lerpScl) * sm::Matrix::CreateFromQuaternion(lerpRot) * sm::Matrix::CreateTranslation(lerpPos);

			if (m_bones[i].parentIndex == -1)
				bonePoseAbsolute[i] = bonePoseRelative;
			else
				bonePoseAbsolute[i] = bonePoseRelative * bonePoseAbsolute[m_bones[i].parentIndex];

			m_localMatrices[i] = m_bones[i].inverseBind * bonePoseAbsolute[i];
		}
		bonePoseAbsolute.clear();
	}
}

void RAnimator::UpperLowerbodyAnimation(const EAnimationType& upper, const EAnimationType& lower)
{
	animation_t* upperAnim = &m_animations[upper];
	animation_t* lowerAnim = &m_animations[lower];

	if (upperAnim && lowerAnim)
	{
		std::vector<sm::Matrix> bonePoseAbsolute;
		bonePoseAbsolute.resize(m_bones.size(), sm::Matrix::Identity);
		animation_t* anim = lowerAnim;

		//Go through all the bones
		for (size_t i = 0; i < m_bones.size(); i++)
		{
			if (m_bones[i].name == upperAnim->devideBone)
			{
				anim = upperAnim;
			}

			sm::Matrix bonePoseRelative = anim->animation->GetMatrix(m_bones[i].name, anim->frameTimer, anim->lastKeys[m_bones[i].name].keys, m_useInterpolation);

			if (m_bones[i].parentIndex == -1)
				bonePoseAbsolute[i] = bonePoseRelative;
			else
				bonePoseAbsolute[i] = bonePoseRelative * bonePoseAbsolute[m_bones[i].parentIndex];

			m_localMatrices[i] = m_bones[i].inverseBind * bonePoseAbsolute[i];
		}
		bonePoseAbsolute.clear();		
	}
}

void RAnimator::BlendUpperBodyAnimations(const EAnimationType& state1, const EAnimationType& state2, const EAnimationType& upper)
{
	animation_t* anim1		= &m_animations[state1];	//Current
	animation_t* anim2		= &m_animations[state2];	//Next
	animation_t* upperAnim	= &m_animations[upper];

	if (anim1 && anim2 && upperAnim)
	{
		std::vector<sm::Matrix> bonePoseAbsolute;
		bonePoseAbsolute.resize(m_bones.size(), sm::Matrix::Identity);
		bool checkUpper = false;

		for (size_t i = 0; i < m_bones.size(); i++)
		{
			if (m_bones[i].name == upperAnim->devideBone)
			{
				checkUpper = true;
			}

			sm::Matrix bonePoseRelative;
			if (!checkUpper)
			{
				//From 
				sm::Vector3 pos1 = anim1->animation->GetPosition(m_bones[i].name, anim1->frameTimer, anim1->lastKeys[m_bones[i].name].keys[0], m_useInterpolation);
				sm::Vector3 scl1 = anim1->animation->GetScale(m_bones[i].name, anim1->frameTimer, anim1->lastKeys[m_bones[i].name].keys[1], m_useInterpolation);
				sm::Quaternion rot1 = anim1->animation->GetRotation(m_bones[i].name, anim1->frameTimer, anim1->lastKeys[m_bones[i].name].keys[2], m_useInterpolation);
				//To
				sm::Vector3 pos2 = anim2->animation->GetPosition(m_bones[i].name, anim2->frameTimer, anim2->lastKeys[m_bones[i].name].keys[0], m_useInterpolation);
				sm::Vector3 scl2 = anim2->animation->GetScale(m_bones[i].name, anim2->frameTimer, anim2->lastKeys[m_bones[i].name].keys[1], m_useInterpolation);
				sm::Quaternion rot2 = anim2->animation->GetRotation(m_bones[i].name, anim2->frameTimer, anim2->lastKeys[m_bones[i].name].keys[2], m_useInterpolation);

				float lerpTime = float(m_animations[state1].blendTimer / m_blendStates.at({ state1, state2 }));
				sm::Vector3 lerpPos = sm::Vector3::Lerp(pos1, pos2, lerpTime);
				sm::Vector3 lerpScl = sm::Vector3::Lerp(scl1, scl2, lerpTime);
				sm::Quaternion lerpRot = sm::Quaternion::Slerp(rot1, rot2, lerpTime);
				lerpRot.Normalize();

				bonePoseRelative = sm::Matrix::CreateScale(lerpScl) * sm::Matrix::CreateFromQuaternion(lerpRot) * sm::Matrix::CreateTranslation(lerpPos);
			}
			else
			{
				bonePoseRelative = upperAnim->animation->GetMatrix(m_bones[i].name, upperAnim->frameTimer, upperAnim->lastKeys[m_bones[i].name].keys, m_useInterpolation);
			}

			if (m_bones[i].parentIndex == -1)
				bonePoseAbsolute[i] = bonePoseRelative;
			else
				bonePoseAbsolute[i] = bonePoseRelative * bonePoseAbsolute[m_bones[i].parentIndex];

			m_localMatrices[i] = m_bones[i].inverseBind * bonePoseAbsolute[i];
		}
		bonePoseAbsolute.clear();
	}


}

void RAnimator::SwapAnimationState()
{
	ResetAnimation(m_currentState);
	m_currentState = m_nextState;
	m_nextState = EAnimationType::NONE;
}

EAnimationCode& RAnimator::GetAnimationCode() const
{
	EAnimationCode codetype = EAnimationCode::NONE;

	enum bitstate
	{
		current = 0x01,
		next	= 0x02,
		upper	= 0x04
	};
	UINT state = 0;

	if (m_currentState	!= EAnimationType::NONE)
		state |= bitstate::current;
	if (m_nextState		!= EAnimationType::NONE)
		state |= bitstate::next;
	if (m_upperState	!= EAnimationType::NONE)
		state |= bitstate::upper;

	switch (state)
	{
	case 1:	//Current
		codetype = EAnimationCode::ONE_ANIM;
		break;
	case 3:	//Current + Next
		codetype = EAnimationCode::TWO_ANIM_BLEND;
		break;
	case 5: //Current + Upper
		codetype = EAnimationCode::TWO_ANIM_UPPER_LOWER;
		break;
	case 7: //Current + Next + Upper
		codetype = EAnimationCode::THREE_ANIM_UPPER_LOWER_BLEND;
		break;
	default:
		break;
	};

	return codetype;
}

bool RAnimator::Create(const std::string& filename)
{
	std::ifstream readfile(ANIMATORPATH + filename);
	std::string line;

	/*
		Load in skeleton and animations from custom animator format.
	*/
	while (std::getline(readfile, line))
	{
		std::stringstream ss(line);
		std::string keyword;
		if (ss >> keyword)
		{
			if (keyword == "skel")
			{
				std::string modelName;
				if (ss >> modelName)
				{
					std::shared_ptr<RModel> model = ResourceManager::Get().GetResource<RModel>(modelName);
					if (model)
					{
						if (!LoadSkeleton(model->GetSkeleton()))
						{
#ifdef _DEBUG
							LOG_WARNING("Animator %s could not load in skeleton...", filename.c_str());
#endif // _DEBUG
							return false;
						}
					}
				}
			}
			else if (keyword == "anim")
			{
				std::string key;
				std::string animName;
				if (ss >> key >> animName)
				{
					EAnimationType animType = StringToAnimationType(key);
					if (animType != EAnimationType::NONE)
					{
						std::shared_ptr<RAnimation> animation = ResourceManager::Get().CopyResource<RAnimation>(animName, true);
						if (animation)
						{
							animation_t animStruct;
							animStruct.animation = animation;
							m_animations[animType] = animStruct;
						}
					}
				}
			}
			else if (keyword == "loopable")
			{
				std::string key;
				bool option;
				if (ss >> key >> option)
				{
					EAnimationType animType = StringToAnimationType(key);
					if (m_animations.find(animType) != m_animations.end())
					{
						m_animations.at(animType).animation->SetLoopable(option);
					}
				}
			}
			else if (keyword == "tickSpeed")
			{
				std::string key;
				double speed;
				if (ss >> key >> speed)
				{
					EAnimationType animType = StringToAnimationType(key);
					if (m_animations.find(animType) != m_animations.end())
					{
						m_animations.at(animType).animation->SetTicksPerFrame(speed);
					}
				}
			}
			else if (keyword == "blendstate")
			{
				std::string key1;
				std::string key2;
				double blendtime = 0;
				if (ss >> key1 >> key2 >> blendtime)
				{
					EAnimationType fromType = StringToAnimationType(key1);
					EAnimationType toType = StringToAnimationType(key2);

					if (fromType != EAnimationType::NONE && toType != EAnimationType::NONE)
					{
						m_blendStates[{fromType, toType}] = blendtime;
					}
				}
			}
			else if (keyword == "upperbody")
			{
				std::string key;
				std::string devideBone;
				if (ss >> key >> devideBone)
				{
					EAnimationType type = StringToAnimationType(key);
					if (m_animations.find(type) != m_animations.end())
					{
						m_animations.at(type).isUpperBody = true;
						m_animations.at(type).devideBone = devideBone;
					}
				}
			}
		}
	}

	readfile.close();
	return true;
}

bool RAnimator::ChangeAnimation(const EAnimationType& type)
{
	bool queueSuccess = false;

	//Check if animation exist
	if (m_animations.find(type) != m_animations.end())
	{
		if (m_currentState == EAnimationType::DEAD)
		{
			std::cout << "New state was: " << (UINT)type << std::endl;
		}


		//No current
		if (m_currentState == EAnimationType::NONE)
		{
			m_currentState = type;
		}
		//Check if we are going to place in the queue
		else
		{
			if (m_currentState != type)
			{
				if (m_animations.at(type).isUpperBody)
				{
					m_upperState = type;
					queueSuccess = true;
				}
				else
				{
					if (m_nextState == EAnimationType::NONE)
					{
						m_nextState = type;
						queueSuccess = true;
					}
				}
			}
			//Else: Already in this state
		}
	}

	return queueSuccess;
}

const EAnimationType& RAnimator::GetCurrentState() const
{
	return m_currentState;
}

const sm::Matrix RAnimator::GetLocalMatrix(const std::string& bonename) const
{
	sm::Matrix bonematrix = sm::Matrix::Identity;

	//Find the bone and what matrix it has
	if (m_nameToBone.find(bonename) != m_nameToBone.end())
	{
		UINT index = m_nameToBone.at(bonename);
		bonematrix = m_localMatrices[index];
	}
	return bonematrix;
}

void RAnimator::Update()
{
	PROFILE_FUNCTION();

	//Need to have a skeleton
	//if (!m_bones.empty())
	//{
	//	//if current



	//}




	//UPPERLOWER WORKED OKAY
	//Need to have a skeleton
	//if (!m_bones.empty())
	//{
	//	if (m_currentState != EAnimationType::NONE)
	//	{
	//		UpdateTime(m_currentState);

	//		if (m_nextState != EAnimationType::NONE)
	//		{
	//			UpdateTime(m_nextState);

	//			//Update blendtimer
	//			double blendDuration = 0;
	//			if (m_blendStates.find({ m_currentState, m_nextState }) != m_blendStates.end())
	//				blendDuration = m_blendStates.at({ m_currentState, m_nextState });

	//			m_animations[m_currentState].blendTimer += Stats::Get().GetUpdateTime();

	//			if (m_upperState != EAnimationType::NONE)
	//			{
	//				UpdateTime(m_upperState);

	//				if (m_animations[m_currentState].blendTimer < blendDuration)
	//					BlendUpperBodyAnimations(m_currentState, m_nextState, m_upperState);
	//				else
	//					SwapAnimationState();
	//			}
	//			else
	//			{
	//				//Regular blending between current and next
	//				if (m_animations[m_currentState].blendTimer < blendDuration)
	//					BlendAnimations(m_currentState, m_nextState);
	//				else
	//					SwapAnimationState();
	//			}
	//		}
	//		else if (m_upperState != EAnimationType::NONE)
	//		{
	//			if (UpdateTime(m_upperState))
	//			{
	//				UpperLowerbodyAnimation(m_upperState, m_currentState);
	//			}
	//			else
	//			{
	//				ResetAnimation(m_upperState);
	//				m_upperState = EAnimationType::NONE;
	//			}
	//		}
	//		else
	//		{
	//			//Regular
	//			RegularAnimation(m_currentState);
	//		}
	//	}

	//	//Check status of the animations? Anyone reached the end? what to do with it?
	//}


	/*
			OLD - WORKED FINE, BUT NOT WITH THE NEW	
	*/
	//Needs a skeleton and a current animation
	if (!m_bones.empty() && m_currentState != EAnimationType::NONE)
	{
		UpdateTime(m_currentState);

		/*
			Only has to focus on the current animation
		*/
		if (m_nextState == EAnimationType::NONE)
		{
			RegularAnimation(m_currentState);
		}
		/*
			We have two animations queued. 
			Check if we are going to start blending.
		*/
		else
		{
			animation_t* anim1 = &m_animations.at(m_currentState);
			if (anim1)
			{
				//How long we shall blend the animations in seconds
				double blendDuration = 0;
				if (m_blendStates.find({ m_currentState, m_nextState }) != m_blendStates.end())
					blendDuration = m_blendStates.at({ m_currentState, m_nextState });
				
				double startTick = anim1->animation->GetDuration() - blendDuration;

				/*
					Loopable animations:		Can start whenever to blend
					None loopable animations:	Need to wait on the right startpoint
				*/
				if (!anim1->animation->IsLoopable() && anim1->frameTimer < startTick)
				{
					RegularAnimation(m_currentState);
				}
				//Going to do blending
				else
				{
					UpdateTime(m_nextState);
					anim1->blendTimer += Stats::Get().GetUpdateTime();

					//Do blending while we can. When we reached the end we swap animation
					if (anim1->blendTimer < blendDuration)
						BlendAnimations(m_currentState, m_nextState);
					else
						SwapAnimationState();
				}
			}
		}
	}	
}

void RAnimator::Bind()
{
	UpdateStructureBuffer();
	D3D11Core::Get().DeviceContext()->VSSetShaderResources(T2D_BONESLOT, 1, m_bonesSB_RSV.GetAddressOf());
}

void RAnimator::Unbind() const
{
	ID3D11ShaderResourceView* nullSRV = nullptr;
	D3D11Core::Get().DeviceContext()->VSSetShaderResources(T2D_BONESLOT, 1, &nullSRV);
}
