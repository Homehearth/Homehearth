#include "EnginePCH.h"
#include "RAnimator.h"

RAnimator::RAnimator()
{
	m_useInterpolation	= true;
	m_currentState		= EAnimationType::NONE;
	m_blendState		= EAnimationType::NONE;
	m_upperState		= EAnimationType::NONE;
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

	while (!m_queue.empty())
		m_queue.pop();
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
		{"PRIMARY",				EAnimationType::PRIMARY},
		{"SECONDARY",			EAnimationType::SECONDARY},
		{"ESCAPE",				EAnimationType::ESCAPE},
		{"PLACE_DEFENCE",		EAnimationType::PLACE_DEFENCE},
		{"TAKE_DAMAGE",			EAnimationType::TAKE_DAMAGE},
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
		anim->frameTimer	= 0.f;
		anim->lastTick		= 0.f;

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
		float tick = anim->animation->GetTicksPerFrame() * Stats::Get().GetUpdateTime();

		if (tick != anim->lastTick)
		{
			anim->lastTick		= tick;
			anim->frameTimer	+= tick;

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
					else if (m_upperState == type)
					{
						m_upperState = EAnimationType::NONE;
					}
				}
			}
			else
			{
				updateSuccess = true;
			}
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
			//Avoid devide by 0
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
			}
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
				std::vector<sm::Matrix> bonePoseAbsolute;
				bonePoseAbsolute.resize(m_bones.size(), sm::Matrix::Identity);
				anim = lowerAnim;
				std::string bonename = m_animations.at(m_upperState).upperbodybone;

				//Go through all the bones
				for (size_t i = 0; i < m_bones.size(); i++)
				{
					std::string name = m_bones[i].name;
					sm::Matrix bonePoseRelative;

					//Swap to the other animation
					if (name == bonename)
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
		if (UpdateTime(m_currentState))
		{
			if (UpdateTime(m_blendState))
			{
				if (UpdateTime(m_upperState))
				{
					bool keepGoing = true;
					float lerpTime;

					if (UpdateBlendTime(m_currentState, m_blendState, lerpTime))
					{
						std::vector<sm::Matrix> bonePoseAbsolute;
						bonePoseAbsolute.resize(m_bones.size(), sm::Matrix::Identity);
						std::string bonename = m_animations.at(m_upperState).upperbodybone;
						bool switched = false;

						for (size_t i = 0; i < m_bones.size(); i++)
						{
							std::string name = m_bones[i].name;

							if (name == bonename)
								switched = true;

							sm::Matrix bonePoseRelative;
							if (switched)
							{
								bonePoseRelative		= upperAnim->animation->GetMatrix(name, upperAnim->frameTimer, upperAnim->lastKeys[name].keys, m_useInterpolation);
							}
							else
							{
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

								bonePoseRelative		= sm::Matrix::CreateScale(lerpScl) * sm::Matrix::CreateFromQuaternion(lerpRot) * sm::Matrix::CreateTranslation(lerpPos);
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
		blend	= 0x02,
		upper	= 0x04
	};
	UINT state = 0;

	if (m_currentState	!= EAnimationType::NONE)
		state |= bitstate::current;
	if (m_blendState != EAnimationType::NONE)
		state |= bitstate::blend;
	if (m_upperState	!= EAnimationType::NONE)
		state |= bitstate::upper;

	switch (state)
	{
	case 1:	//Current
		codetype = EAnimStatus::ONE_ANIM;
		break;
	case 3:	//Current + Blend
		codetype = EAnimStatus::TWO_ANIM_BLEND;
		break;
	case 5: //Current + Upper
		codetype = EAnimStatus::TWO_ANIM_UPPER_LOWER;
		break;
	case 7: //Current + Blend + Upper
		codetype = EAnimStatus::THREE_ANIM_UPPER_LOWER_BLEND;
		break;
	default:
		break;
	};

	return codetype;
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
	if (!m_queue.empty())
	{
		EAnimationType queued = m_queue.front();

		switch (GetAnimStatus())
		{
		//No animation
		case EAnimStatus::NONE:
		{
			m_currentState = queued;
			m_queue.pop();
			break;
		}
		//Currently running one animation
		case EAnimStatus::ONE_ANIM:
		{
			//Transition to this animation exist
			if (m_states.find({ m_currentState, queued }) != m_states.end())
			{
				//Wait for the perfect movement for quueuing up an animation after oneshot animation
				if (!m_animations.at(m_currentState).animation->IsLoopable())
				{
					if (ReadyToBlend(m_currentState, queued))
					{
						m_blendState = queued;
						m_queue.pop();
					}
					//Keep waiting
				}
				else
				{
					//Upper part animation - Do not work on idle
					if (!m_animations.at(queued).upperbodybone.empty() && m_currentState != EAnimationType::IDLE)
					{
						m_upperState = queued;
						m_queue.pop();
					}
					//Loopable animation, just queue up the next
					else
					{
						m_blendState = queued;
						m_queue.pop();
					}
				}
			}
			//No transition - just pop and ignore this animation...
			else
			{
				m_queue.pop();
			}
			break;
		}
		//Blending two animations
		case EAnimStatus::TWO_ANIM_BLEND:
		{
			//Swap direction of blend if needed
			if (m_animations.at(m_currentState).animation->IsLoopable() &&
				m_animations.at(m_blendState).animation->IsLoopable())
			{
				//Blend back to current again
				if (m_currentState == queued)
				{
					m_blendDir = false;
					m_queue.pop();
				}
				//Blend toward the next animation
				else if (m_blendState == queued)
				{
					m_blendDir = true;
					m_queue.pop();
				}
				//Cancel the blending with the queued up one
				else
				{
					//Has a upperbody bone, then okay to queue it to upperstate
					if (!m_animations.at(queued).upperbodybone.empty())
					{
						if (m_upperState == EAnimationType::NONE)
						{
							m_upperState = queued;
							m_queue.pop();
						}
					}
				}
			}
			break;
		}
		//In current + upper:			ignore
		//In current + upper + blend:	ignore
		default:
			break;
		}

		//Queue is to big - could be stuck...
		while (m_queue.size() > 3)
		{
			m_queue.pop();
		}
	}
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
						m_states[{fromType, toType}] = { 0.f, blendtime };
					}
				}
			}
			else if (keyword == "upperbody")
			{
				std::string key;
				std::string bonename;
				if (ss >> key >> bonename)
				{
					EAnimationType animType = StringToAnimationType(key);
					if (m_animations.find(animType) != m_animations.end())
					{
						m_animations.at(animType).upperbodybone = bonename;
					}
				}
			}
			else if (keyword == "stayAtEnd")
			{
				std::string key;
				bool stayAtEnd = false;
				if (ss >> key >> stayAtEnd)
				{
					EAnimationType animType = StringToAnimationType(key);
					if (m_animations.find(animType) != m_animations.end())
					{
						m_animations.at(animType).stayAtEnd = stayAtEnd;
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
	if (m_animations.size() == 8)
		std::cout << "Incoming" << (UINT)type << std::endl;

	//Check if animation exist
	if (m_animations.find(type) != m_animations.end())
	{
		//Queue up the animation
		m_queue.push(type);
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
		//Check if there was anyone in queue
		CheckQueue();

		switch (GetAnimStatus())
		{
		case EAnimStatus::ONE_ANIM:
			StandardAnim();
			break;
		case EAnimStatus::TWO_ANIM_BLEND:
			BlendTwoAnims();
			break;
		case EAnimStatus::TWO_ANIM_UPPER_LOWER:
			UpperLowerAnims();
			break;
		case EAnimStatus::THREE_ANIM_UPPER_LOWER_BLEND:
			BlendUpperLowerAnims();
			break;
		default:
			break;
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
