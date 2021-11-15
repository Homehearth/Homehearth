#include "EnginePCH.h"
#include "RAnimator.h"

RAnimator::RAnimator()
{
	m_lastTick			= 0;
	m_useInterpolation	= true;
	m_currentType		= AnimationType::NONE;
	m_nextType			= AnimationType::NONE;
	m_defaultType		= AnimationType::NONE;
}

RAnimator::~RAnimator()
{
	m_bones.clear();
	m_finalMatrices.clear();
	m_animations.clear();
}

bool RAnimator::LoadSkeleton(const std::vector<bone_t>& skeleton)
{
	//Clear previous bones and matrices if needed
	m_finalMatrices.clear();
	m_bones.clear();

	bool loaded = false;
	if (!skeleton.empty())
	{
		for (size_t i = 0; i < skeleton.size(); i++)
		{
			bone_keyFrames_t bone;
			bone.name = skeleton[i].name;
			bone.inverseBind = skeleton[i].inverseBind;
			bone.parentIndex = skeleton[i].parentIndex;
			m_bones.push_back(bone);
		}

		m_finalMatrices.resize(m_bones.size(), sm::Matrix::Identity);

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
	auto iterator = m_animations.find(m_currentType);

	//Check that it is okay to edit this animation
	if (iterator != m_animations.end())
	{
		iterator->second.frameTimer = std::rand() / m_animations[m_currentType].animation->GetDuraction();
	}
}

bool RAnimator::CreateBonesSB()
{
	UINT nrOfMatrices = static_cast<UINT>(m_finalMatrices.size());

	//Can't create a buffer if there is no bones...
	if (m_finalMatrices.empty())
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
	memcpy(submap.pData, &m_finalMatrices[0], sizeof(sm::Matrix) * m_finalMatrices.size());
	D3D11Core::Get().DeviceContext()->Unmap(m_bonesSB_Buffer.Get(), 0);
}

AnimationType RAnimator::StringToAnimationType(const std::string& name) const
{
	AnimationType type = AnimationType::NONE;
	
	std::unordered_map<std::string, AnimationType> animationMap
	{
		{"NONE",				AnimationType::NONE},
		{"IDLE",				AnimationType::IDLE},
		{"WALK",				AnimationType::WALK},
		{"RUN",					AnimationType::RUN},
		{"PRIMARY_ATTACK",		AnimationType::PRIMARY_ATTACK},
		{"SECONDARY_ATTACK",	AnimationType::SECONDARY_ATTACK},
		{"ABILITY1",			AnimationType::ABILITY1},
		{"ABILITY2",			AnimationType::ABILITY2},
		{"ABILITY3",			AnimationType::ABILITY3},
		{"ABILITY4",			AnimationType::ABILITY4},
		{"TAKE_DAMAGE",			AnimationType::TAKE_DAMAGE},
		{"PLACE_DEFENCE",		AnimationType::PLACE_DEFENCE}
	};

	//Search for the keyword
	if (animationMap.find(name) != animationMap.end())
	{
		type = animationMap[name];
	}
	return  type;
}

void RAnimator::ResetLastKeys()
{
	//Reset safespot in bones
	for (size_t i = 0; i < m_bones.size(); i++)
	{
		m_bones[i].lastKeys = { 0, 0, 0 };
	}
}

void RAnimator::ResetAnimation(const AnimationType& type)
{
	if (m_animations.find(type) != m_animations.end())
	{
		animation_t* anim = &m_animations[type];
		anim->frameTimer = 0;
		anim->blendTimer = 0;
	}
}

bool RAnimator::UpdateTime()
{
	bool shouldUpdate = true;
	animation_t* current = &m_animations[m_currentType];

	if (current)
	{
		double tick = current->animation->GetTicksPerFrame() * Stats::Get().GetUpdateTime();

		if (m_lastTick != tick)
		{
			m_lastTick = tick;
			current->frameTimer += tick;

			if (current->frameTimer >= current->animation->GetDuraction())
			{
				if (current->animation->IsLoopable())
				{
					current->frameTimer = fmod(current->frameTimer, current->animation->GetDuraction());
				}
				else
				{
					//Go to next state
					shouldUpdate = false;
					ChangeAnimation(m_defaultType);
				}
			}
		}
		else
		{
			shouldUpdate = false;
		}
	}
	else
	{
		shouldUpdate = false;
	}
	return shouldUpdate;
}

bool RAnimator::Create(const std::string& filename)
{
	std::ifstream readfile(ANIMATORPATH + filename);
	std::string line;
	std::shared_ptr<RAnimation> lastAnimation;

	/*
		Load in skeleton and animations from custom animator format.
	*/
	while (std::getline(readfile, line))
	{
		std::stringstream ss(line);
		std::string keyword;
		ss >> keyword;

		if (keyword == "skel")
		{
			std::string modelName;
			ss >> modelName;

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
		else if (keyword == "anim")
		{
			std::string key;
			std::string animName;
			ss >> key >> animName;

			AnimationType animType = StringToAnimationType(key);
			if (animType != AnimationType::NONE)
			{
				std::shared_ptr<RAnimation> animation = ResourceManager::Get().GetResource<RAnimation>(animName);
				if (animation)
				{
					animation_t animStruct;
					animStruct.animation = animation;
					m_animations[animType] = animStruct;
					lastAnimation = animation;
				}				
			}
		}
		else if (keyword == "defaultAnim")
		{
			std::string key;
			ss >> key;

			AnimationType animType = StringToAnimationType(key);
			if (m_animations.find(animType) != m_animations.end())
			{
				m_currentType = animType;
				m_defaultType = animType;
			}
		}
		else if (keyword == "loopable")
		{
			bool option = false;
			ss >> option;
			if (lastAnimation)
				lastAnimation->SetLoopable(option);
		
		}
		else if (keyword == "tickSpeed")
		{
			double speed = 0;
			ss >> speed;
			if (lastAnimation)
				lastAnimation->SetTicksPerFrame(speed);
		}
	}

	readfile.close();

	return true;
}

bool RAnimator::ChangeAnimation(const AnimationType& type)
{
	bool foundAnim = false;

	//Don't change if we already are in this state.
	if (m_currentType != type)
	{
		//Change animation if it exist
		if (m_animations.find(type) != m_animations.end())
		{
			ResetAnimation(m_currentType);
			ResetAnimation(type);
			m_currentType = type;
			ResetLastKeys();
			foundAnim = true;
		}
		if (type == AnimationType::NONE)
		{
			ResetAnimation(m_currentType);
			m_currentType = type;
			ResetLastKeys();
		}
	}
	return foundAnim;
}

const AnimationType& RAnimator::GetCurrentState() const
{
	return m_currentType;
}

void RAnimator::Update()
{
	/*
		TODO: Need to be cleaned up
	*/

	PROFILE_FUNCTION();

	//Needs a skeleton
	if (!m_bones.empty())
	{
		if (m_currentType != AnimationType::NONE)
		{
			//Do blending
			if (m_nextType != AnimationType::NONE)
			{
				//TODO: Make separate function
				//tickCurrent =  ...
				//tickNext = ...
				//check the blendtimer - where are we
			}
			//Do regular
			else
			{
				if (UpdateTime())
				{
					std::vector<sm::Matrix> modelMatrices;
					modelMatrices.resize(m_bones.size(), sm::Matrix::Identity);

					for (size_t i = 0; i < m_bones.size(); i++)
					{
						sm::Matrix localMatrix = m_animations[m_currentType].animation->GetMatrix(m_bones[i].name, m_animations[m_currentType].frameTimer, m_bones[i].lastKeys, m_useInterpolation);

						if (m_bones[i].parentIndex == -1)
							modelMatrices[i] = localMatrix;
						else
							modelMatrices[i] = localMatrix * modelMatrices[m_bones[i].parentIndex];

						m_finalMatrices[i] = m_bones[i].inverseBind * modelMatrices[i];
					}

					modelMatrices.clear();
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
