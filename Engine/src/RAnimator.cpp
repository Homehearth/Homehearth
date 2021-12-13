#include "EnginePCH.h"
#include "RAnimator.h"

RAnimator::RAnimator()
{
	m_useInterpolation	= true;
	m_currentState		= EAnimationType::NONE;
	m_blendState		= EAnimationType::NONE;
	m_upperState		= EAnimationType::NONE;
	m_queueState		= EAnimationType::NONE;
	m_blendDir			= true;
}

RAnimator::~RAnimator()
{
	for (auto& anim : m_animations)
	{
		anim.second.lastKeys.clear();
	}
	m_bones.clear();
	m_nameToBone.clear();
	m_animations.clear();
	m_states.clear();
	m_localMatrices.clear();
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

		for (auto& key : anim->lastKeys)
			key.second = { 0,0,0 };
	}
}

bool RAnimator::UpdateTime(const EAnimationType& type)
{
	bool updateSuccess = false;
	animation_t* anim = &m_animations[type];
	if (anim)
	{
		anim->frameTimer += anim->animation->GetTicksPerFrame() * Stats::Get().GetUpdateTime();

		//Reached end
		if (anim->frameTimer >= anim->animation->GetDuration())
		{
			if (anim->animation->IsLoopable())
			{
				anim->frameTimer = fmod(anim->frameTimer, anim->animation->GetDuration());
				updateSuccess = true;
			}
			else
			{
				ResetAnimation(type);
				if (m_currentState == type)
				{
					m_currentState = m_blendState;
					m_blendState = EAnimationType::NONE;
				}
				else if (m_blendState == type)
				{
					m_blendState = EAnimationType::NONE;
				}
			}
		}
		else
		{
			updateSuccess = true;
		}
	}
	return updateSuccess;
}

bool RAnimator::UpdateBlendTime(const EAnimationType& from, const EAnimationType& to, float& lerpTime)
{
	lerpTime = 0;

	if (m_states.find({ from, to }) != m_states.end())
	{
		float blendDuration = m_states.at({ from, to }).blendDuration;

		if (m_blendDir)
			m_states.at({ from, to }).blendTimer += Stats::Get().GetUpdateTime();
		else
			m_states.at({ from, to }).blendTimer -= Stats::Get().GetUpdateTime();

		float blendTime = m_states.at({ from, to }).blendTimer;
		
		//Reset blendtimer
		if (blendTime > blendDuration)
		{
			m_states.at({ from, to }).blendTimer = 0;
			m_blendDir = true;
			ResetAnimation(from);
			m_currentState = m_blendState;
			m_blendState = EAnimationType::NONE;
			return false;
		}
		else if (blendTime < 0)
		{
			m_states.at({ from, to }).blendTimer = 0;
			m_blendDir = true;
			ResetAnimation(to);
			m_blendState = EAnimationType::NONE;
			return false;
		}
		//Successfully updated the blendtime
		else
		{
			if (blendDuration > 0)
				lerpTime = float(blendTime / blendDuration);
			return true;
		}
	}
	
	return false;
}

void RAnimator::StandardAnim()
{
	animation_t* anim = &m_animations[m_currentState];
	
	if (anim)
	{
		/*if (m_animations.size() == 7)
			if (m_currentState == EAnimationType::PRIMARY_ATTACK)
				std::cout << "Playing: primary" << std::endl;*/

		//Only update when we get a new timestamp
		if (UpdateTime(m_currentState))
		{
			std::vector<sm::Matrix> bonePoseAbsolute;
			bonePoseAbsolute.resize(m_bones.size(), sm::Matrix::Identity);
			std::string name = "";

			//Go through all the bones
			for (size_t i = 0; i < m_bones.size(); i++)
			{
				name = m_bones[i].name;
				sm::Matrix bonePoseRelative = anim->animation->GetMatrix(name, anim->frameTimer, anim->lastKeys[name].keys, m_useInterpolation);

				if (m_bones[i].parentIndex == -1)
					bonePoseAbsolute[i] = bonePoseRelative;
				else
					bonePoseAbsolute[i] = bonePoseRelative * bonePoseAbsolute[m_bones[i].parentIndex];

				m_localMatrices[i] = m_bones[i].inverseBind * bonePoseAbsolute[i];
			}
			bonePoseAbsolute.clear();
		}
		else
		{
			std::cout << "Reached end" << std::endl;
		}
	}
}

void RAnimator::BlendTwoAnims()
{
	animation_t* anim1 = &m_animations[m_currentState];
	animation_t* anim2 = &m_animations[m_blendState];

	if (anim1 && anim2)
	{
		if (UpdateTime(m_currentState))
		{
			if (UpdateTime(m_blendState))
			{
				float lerpTime;
				if (UpdateBlendTime(m_currentState, m_blendState, lerpTime))
				{
					std::vector<sm::Matrix> bonePoseAbsolute;
					bonePoseAbsolute.resize(m_bones.size(), sm::Matrix::Identity);
					std::string name = "";
					/*if (m_animations.size() == 7)
					{
						std::cout << "Current: " << 1.f - lerpTime << " | " << "Blending: " << lerpTime << std::endl;
					}*/

					for (size_t i = 0; i < m_bones.size(); i++)
					{
						name = m_bones[i].name;
						//From 
						sm::Vector3 pos1		= anim1->animation->GetPosition(name, anim1->frameTimer, anim1->lastKeys[name].keys[0], m_useInterpolation);
						sm::Vector3 scl1		= anim1->animation->GetScale(	name, anim1->frameTimer, anim1->lastKeys[name].keys[1], m_useInterpolation);
						sm::Quaternion rot1		= anim1->animation->GetRotation(name, anim1->frameTimer, anim1->lastKeys[name].keys[2], m_useInterpolation);
						//To
						sm::Vector3 pos2		= anim2->animation->GetPosition(name, anim2->frameTimer, anim2->lastKeys[name].keys[0], m_useInterpolation);
						sm::Vector3 scl2		= anim2->animation->GetScale(	name, anim2->frameTimer, anim2->lastKeys[name].keys[1], m_useInterpolation);
						sm::Quaternion rot2		= anim2->animation->GetRotation(name, anim2->frameTimer, anim2->lastKeys[name].keys[2], m_useInterpolation);

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
				else
				{
					//std::cout << "Reached end for blending" << std::endl;
				}
			}
			else
			{
				std::cout << "Reached end on blendstate" << std::endl;
			}
		}
		else
		{
			std::cout << "Reached end on currentstate: " << (UINT)m_currentState << " blending toward: " << (UINT)m_blendState << std::endl;
		}
	}
}

void RAnimator::UpperLowerAnims()
{
	animation_t* lowerAnim = &m_animations[m_currentState];
	animation_t* upperAnim = &m_animations[m_upperState];
	animation_t* anim;	//Current
	
	if (lowerAnim && upperAnim)
	{
		if (UpdateTime(m_currentState))
		{
			if (UpdateTime(m_upperState))
			{
				std::string devideBone = m_states.at({ m_currentState, m_upperState }).devidebone;
				std::vector<sm::Matrix> bonePoseAbsolute;
				bonePoseAbsolute.resize(m_bones.size(), sm::Matrix::Identity);
				anim = lowerAnim;

				//Go through all the bones
				for (size_t i = 0; i < m_bones.size(); i++)
				{
					std::string name = m_bones[i].name;
					sm::Matrix bonePoseRelative;

					//Swap to the other animation
					if (name == devideBone)
						anim = upperAnim;

					bonePoseRelative = anim->animation->GetMatrix(name, anim->frameTimer, anim->lastKeys[name].keys, m_useInterpolation);

					if (m_bones[i].parentIndex == -1)
						bonePoseAbsolute[i] = bonePoseRelative;
					else
						bonePoseAbsolute[i] = bonePoseRelative * bonePoseAbsolute[m_bones[i].parentIndex];

					m_localMatrices[i] = m_bones[i].inverseBind * bonePoseAbsolute[i];
				}
				bonePoseAbsolute.clear();
			}
			else
			{
				std::cout << "Reached end on upperstate" << std::endl;
			}
		}
		else
		{ 
			std::cout << "Reached end on currentstate" << std::endl;
		}

	}
}

void RAnimator::BlendUpperLowerAnims()
{
	animation_t* anim1		= &m_animations[m_currentState];
	animation_t* anim2		= &m_animations[m_blendState];
	animation_t* upperAnim	= &m_animations[m_upperState];
	
	if (anim1 && anim2 && upperAnim)
	{
		if (UpdateTime(m_currentState) && UpdateTime(m_blendState) && UpdateTime(m_upperState))
		{
			bool keepGoing = true;
			float lerpTime1;
			float lerpTime2;

			if (!UpdateBlendTime(m_currentState, m_blendState, lerpTime1))
			{
				ResetAnimation(m_currentState);
				keepGoing = false;
			}
			if (!UpdateBlendTime(m_currentState, m_upperState, lerpTime2))
			{
				ResetAnimation(m_upperState);
				keepGoing = false;
			}

			if (keepGoing)
			{
				std::string devideBone = m_states.at({ m_currentState, m_upperState }).devidebone;
				std::vector<sm::Matrix> bonePoseAbsolute;
				bonePoseAbsolute.resize(m_bones.size(), sm::Matrix::Identity);

				for (size_t i = 0; i < m_bones.size(); i++)
				{
					std::string name = m_bones[i].name;

					if (name == devideBone)
					{
						anim2		= upperAnim;
						lerpTime1	= lerpTime2;
						//NOTE: Need to fix the scaling. Different between this two
						//double blendDelta = blendDuration - blendtimer;
					}

					sm::Matrix bonePoseRelative;
					//From 
					sm::Vector3 pos1		= anim1->animation->GetPosition(name, anim1->frameTimer, anim1->lastKeys[name].keys[0], m_useInterpolation);
					sm::Vector3 scl1		= anim1->animation->GetScale(	name, anim1->frameTimer, anim1->lastKeys[name].keys[1], m_useInterpolation);
					sm::Quaternion rot1		= anim1->animation->GetRotation(name, anim1->frameTimer, anim1->lastKeys[name].keys[2], m_useInterpolation);
					//To
					sm::Vector3 pos2		= anim2->animation->GetPosition(name, anim2->frameTimer, anim2->lastKeys[name].keys[0], m_useInterpolation);
					sm::Vector3 scl2		= anim2->animation->GetScale(	name, anim2->frameTimer, anim2->lastKeys[name].keys[1], m_useInterpolation);
					sm::Quaternion rot2		= anim2->animation->GetRotation(name, anim2->frameTimer, anim2->lastKeys[name].keys[2], m_useInterpolation);

					sm::Vector3 lerpPos		= sm::Vector3::Lerp(pos1, pos2, lerpTime1);
					sm::Vector3 lerpScl		= sm::Vector3::Lerp(scl1, scl2, lerpTime1);
					sm::Quaternion lerpRot	= sm::Quaternion::Slerp(rot1, rot2, lerpTime1);
					lerpRot.Normalize();

					bonePoseRelative = sm::Matrix::CreateScale(lerpScl) * sm::Matrix::CreateFromQuaternion(lerpRot) * sm::Matrix::CreateTranslation(lerpPos);

					if (m_bones[i].parentIndex == -1)
						bonePoseAbsolute[i] = bonePoseRelative;
					else
						bonePoseAbsolute[i] = bonePoseRelative * bonePoseAbsolute[m_bones[i].parentIndex];

					m_localMatrices[i] = m_bones[i].inverseBind * bonePoseAbsolute[i];
				}
				bonePoseAbsolute.clear();
			}
		}
	}
}

EAnimStatus RAnimator::GetAnimStatus() const
{
	EAnimStatus codetype = EAnimStatus::NONE;

	enum bitstate
	{
		current = 0x01,
		next	= 0x02,
		upper	= 0x04
	};
	UINT state = 0;

	if (m_currentState	!= EAnimationType::NONE)
		state |= bitstate::current;
	if (m_blendState != EAnimationType::NONE)
		state |= bitstate::next;
	if (m_upperState	!= EAnimationType::NONE)
		state |= bitstate::upper;

	switch (state)
	{
	case 1:	//Current
		codetype = EAnimStatus::ONE_ANIM;
		break;
	case 3:	//Current + Next
		codetype = EAnimStatus::TWO_ANIM_BLEND;
		break;
	case 5: //Current + Upper
		codetype = EAnimStatus::TWO_ANIM_UPPER_LOWER;
		break;
	case 7: //Current + Next + Upper
		codetype = EAnimStatus::THREE_ANIM_UPPER_LOWER_BLEND;
		break;
	default:
		break;
	};

	return codetype;
}

bool RAnimator::IsBlending() const
{
	bool isblending = false;

	if (m_currentState != EAnimationType::NONE &&
		m_blendState != EAnimationType::NONE)
		isblending = true;

	return isblending;
}

bool RAnimator::ReadyToBlend(const EAnimationType& from, const EAnimationType& to) const
{
	bool ready = false;

	float blendDuration = 0.f;
	if (m_states.find({ from, to }) != m_states.end())
		blendDuration = m_states.at({ from, to }).blendDuration;
	
	float duration = m_animations.at(m_currentState).animation->GetDurationInSeconds();
	float startPoint = duration - blendDuration;
	
	//Need to get where it is in the timeline in seconds
	float actualTime = m_animations.at(from).frameTimer / m_animations.at(m_currentState).animation->GetDuration();

	//Startpoint is in seconds
	if (actualTime > startPoint)
		ready = true;

	return ready;
}

void RAnimator::CheckQueue()
{
	if (m_queueState != EAnimationType::NONE)
	{
		/*if (m_animations.size() == 7)
			if (m_currentState == EAnimationType::PRIMARY_ATTACK)
				std::cout << "Stop right there!" << std::endl;*/

		//No animation
		if (m_currentState == EAnimationType::NONE)
		{
			m_currentState  = m_queueState;
			m_queueState = EAnimationType::NONE;
		}
		//Currently running one animation
		else if (!IsBlending())
		{
			if (m_states.find({ m_currentState, m_queueState }) != m_states.end())
			{
				if (!m_animations.at(m_currentState).animation->IsLoopable())
				{
					if (ReadyToBlend(m_currentState, m_queueState))
					{
						m_blendState = m_queueState;
						m_queueState = EAnimationType::NONE;
					}
					//Keep waiting
				}
				else
				{
					m_blendState = m_queueState;
					m_queueState = EAnimationType::NONE;
				}
			}
			else
			{
				m_queueState = EAnimationType::NONE;
			}
		}
		//Blending two animations
		else
		{
			//Swap direction of blend if needed
			if (m_animations.at(m_currentState).animation->IsLoopable()&&
				m_animations.at(m_blendState).animation->IsLoopable())
			{
				//Blend back to current again
				if (m_currentState == m_queueState)
				{
					m_blendDir = false;
					m_queueState = EAnimationType::NONE;
				}
				//Blend toward the next animation
				else if (m_blendState == m_queueState)
				{
					m_blendDir = true;
					m_queueState = EAnimationType::NONE;
				}
			}
		}

		/*if (!m_animations.at(m_queueState).animation->IsLoopable())
		{
			ResetAnimation(m_currentState);
			if (IsBlending())
			{
				ResetAnimation(m_blendState);
				m_blendState = EAnimationType::NONE;
			}

			m_currentState = m_queueState;
			m_queueState = EAnimationType::NONE;
		}
		else
		{

		}*/

		//switch (GetAnimStatus())
		//{
		//	case EAnimStatus::NONE:
		//	{
		//		m_currentState = m_queueState;
		//		m_queueState = EAnimationType::NONE;
		//		break;
		//	}
		//	//Only one animation running for now
		//	case EAnimStatus::ONE_ANIM:
		//	{
		//		//Wait until perfect moment for Oneshot animations
		//		if (!m_animations.at(m_currentState).animation->IsLoopable())
		//		{
		//			float blendDuration = 0.f;
		//			if (m_states.find({ m_currentState, m_queueState }) != m_states.end())
		//			{
		//				blendDuration = m_states.at({ m_currentState, m_queueState }).blendDuration;
		//			}
		//			float duration = m_animations.at(m_currentState).animation->GetDurationInSeconds();
		//			float startPoint = duration - blendDuration;

		//			if (m_animations.at(m_currentState).frameTimer > startPoint)
		//			{
		//				m_blendState = m_queueState;
		//				m_queueState = EAnimationType::NONE;
		//			}
		//		}
		//		else
		//		{
		//			//Blendstate exist
		//			if (m_states.find({ m_currentState, m_queueState }) != m_states.end())
		//			{
		//				m_blendState = m_queueState;
		//				m_queueState = EAnimationType::NONE;
		//			}
		//			//No blendstate override
		//			else
		//			{
		//				ResetAnimation(m_currentState);
		//				m_currentState = m_queueState;
		//				m_queueState = EAnimationType::NONE;
		//			}
		//		}

		//		//Animations that can't be looped is play once animations
		//		/*if (!m_animations.at(m_queueState).animation->IsLoopable())
		//		{
		//			m_currentState	= m_queueState;
		//			m_queueState	= EAnimationType::NONE;
		//		}
		//		else
		//		{
		//			if (m_states.find({ m_currentState, m_queueState }) != m_states.end())
		//			{
		//				m_blendState = m_queueState;
		//				m_queueState = EAnimationType::NONE;
		//			}
		//		}*/
		//		break;
		//	}
		//	case EAnimStatus::TWO_ANIM_BLEND:
		//	{
		//		//Only works for loopable animations
		//		if (m_animations.at(m_currentState).animation->IsLoopable() &&
		//			m_animations.at(m_blendState).animation->IsLoopable())
		//		{
		//			//Blend back to current again
		//			if (m_currentState == m_queueState)
		//			{
		//				m_blendDir = false;
		//				m_queueState = EAnimationType::NONE;
		//			}
		//			//Blend toward the next animation
		//			else if (m_blendState == m_queueState)
		//			{
		//				m_blendDir = true;
		//				m_queueState = EAnimationType::NONE;
		//			}
		//		}
		//		else
		//		{
		//			ResetAnimation(m_blendState);
		//			m_blendDir = true;
		//			m_blendState = m_queueState;
		//			m_queueState = EAnimationType::NONE;
		//		}
		//		break;
		//	}
		//	default:
		//		break;
		//}
	}
}

//void RAnimator::CheckEndedAnims()
//{
//	if (m_currentState != EAnimationType::NONE)
//	{
//		if (m_animations.at(m_currentState).needReset)
//		{
//			ResetAnimation(m_currentState);
//			m_blendDir = true;
//
//			m_currentState = m_blendState;
//			m_blendState = EAnimationType::NONE;
//		}
//	}
//	if (m_blendState != EAnimationType::NONE)
//	{
//		if (m_animations.at(m_blendState).needReset)
//		{
//			ResetAnimation(m_blendState);
//			m_blendState = EAnimationType::NONE;
//			m_blendDir = true;
//		}
//	}
//}


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
				float speed;
				if (ss >> key >> speed)
				{
					EAnimationType animType = StringToAnimationType(key);
					if (m_animations.find(animType) != m_animations.end())
					{
						m_animations.at(animType).animation->SetTicksPerFrame(speed);
					}
				}
			}
			else if (keyword == "state")
			{
				std::string key1;
				std::string key2;
				float		blendtime = 0.f;
				if (ss >> key1 >> key2 >> blendtime)
				{
					EAnimationType fromType = StringToAnimationType(key1);
					EAnimationType toType	= StringToAnimationType(key2);

					if (fromType != EAnimationType::NONE && toType != EAnimationType::NONE)
					{
						if (m_animations.find(fromType) != m_animations.end())
						{
							//Set blendtime to end for animation that will not be looped
							if (!m_animations.at(fromType).animation->IsLoopable())
							{
								float duration = m_animations.at(fromType).animation->GetDurationInSeconds();
								if (blendtime > duration)
									blendtime = duration;
							}
						}

						std::string devidebone = "";
						ss >> devidebone;
						m_states[{fromType, toType}] = { 0.f, blendtime, devidebone };
					}
				}
			}
		}
	}

	readfile.close();
	return true;
}

void RAnimator::ChangeAnimation(const EAnimationType& type)
{
	//Check if animation exist
	if (m_animations.find(type) != m_animations.end())
	{
		m_queueState = type;

		//Get current status of what is set
		//switch (GetAnimStatus())
		//{
		//	//No animation is running
		//	case EAnimStatus::NONE:
		//	{
		//		m_currentState = type;
		//		break;
		//	}
		//	//Only one animation running for now
		//	case EAnimStatus::ONE_ANIM:
		//	{
		//		//Queue up one more?
		//		//Possible to go to this state from current
		//		if (m_states.find({ m_currentState, type }) != m_states.end())
		//		{
		//			if (!m_animations.at(type).animation->IsLoopable())
		//			{
		//				m_currentState = type;
		//			}
		//			/*else
		//			{
		//				m_blendState = type;
		//			}*/


		//			/*if (m_states.at({ m_currentState, type }).devidebone.empty())
		//			{
		//				m_blendState = type;
		//			}
		//			else
		//			{
		//				m_upperState = type;
		//			}*/
		//		}
		//		break;
		//	}
		//	case EAnimStatus::TWO_ANIM_BLEND:
		//	{
		//		//Blend back to current again
		//		if (m_currentState == type)
		//		{
		//			if (m_states.at({m_currentState, m_blendState}).animationCancel)
		//				m_blendDir = false;
		//			 else
		//				m_queueState = type;
		//		}
		//		//Blend toward the next animation
		//		else if (m_blendState == type)
		//		{
		//			if (m_states.at({ m_currentState, m_blendState }).animationCancel)
		//				m_blendDir = true;
		//			else
		//				m_queueState = type;
		//		}
		//		break;
		//	}
		//	default:
		//		break;
		//};


		//DEBUGGING
		//if (m_animations.size() == 7)
		//{
		//	std::cout << "Start - Incoming animation: " << (UINT)type << std::endl;
		//	std::cout << "Current anim: " << (UINT)m_currentState << std::endl;
		//	std::cout << "Blend anim: " << (UINT)m_blendState << std::endl;
		//	//std::cout << "Upper anim: " << (UINT)m_upperState << std::endl;
		//	if (m_blendDir)
		//		std::cout << "Blending toward: " << (UINT)m_blendState << " [+]" << std::endl;
		//	else
		//		std::cout << "Blending toward: " << (UINT)m_currentState << " [-]" << std::endl;
		//	std::cout << "-------------" << std::endl;
		//}
		//DEBUGGING
	}
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
	if (!m_bones.empty())
	{
		CheckQueue();

		if (m_animations.size() == 7)
			if (m_queueState != EAnimationType::NONE)
				std::cout << "Queued: " << (UINT)m_queueState << std::endl;

		switch (GetAnimStatus())
		{
		case EAnimStatus::ONE_ANIM:
			StandardAnim();
			break;
		case EAnimStatus::TWO_ANIM_BLEND:
			/*if (m_currentState == EAnimationType::PRIMARY_ATTACK)
				std::cout << "Fading in primary" << std::endl;
			else if (m_blendState == EAnimationType::PRIMARY_ATTACK)
				std::cout << "Fading out primary" << std::endl;*/

			BlendTwoAnims();
			break;
		case EAnimStatus::TWO_ANIM_UPPER_LOWER:
			UpperLowerAnims();								//temp
			break;
		case EAnimStatus::THREE_ANIM_UPPER_LOWER_BLEND:
			//BlendUpperLowerAnims();						//temp
			break;
		default:
			break;
		}

		//Check if any animation has ended? Reset? 
		//CheckEndedAnims();
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
