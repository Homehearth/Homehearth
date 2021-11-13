#include "EnginePCH.h"
#include "RAnimator.h"

RAnimator::RAnimator()
{
	m_currentFrameTime = 0;
	m_useInterpolation = true;
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
	if (m_currentAnim)
	{
		m_currentFrameTime = std::rand() / m_currentAnim->GetDuraction();
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

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem				= &m_finalMatrices[0];

	HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&desc, &data, m_bonesSB_Buffer.GetAddressOf());
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

AnimationType RAnimator::GetAnimationType(const std::string& name) const
{


	return AnimationType();
}

void RAnimator::ResetTime()
{
	//Reset frametime
	m_currentFrameTime = 0;

	//Reset safespot in bones
	for (size_t i = 0; i < m_bones.size(); i++)
	{
		m_bones[i].lastKeys = { 0, 0, 0 };
	}
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

			std::shared_ptr<RAnimation> animation = ResourceManager::Get().GetResource<RAnimation>(animName);
			if (animation)
			{
				m_animations[key] = animation;
				lastAnimation = animation;
			}
		}
		else if (keyword == "defaultAnim")
		{
			std::string key;
			ss >> key;
			if (m_animations.find(key) != m_animations.end())
			{
				m_defaultAnim = m_animations[key];
				m_currentAnim = m_defaultAnim;
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

bool RAnimator::ChangeAnimation(const std::string& name)
{
	auto iterator = m_animations.find(name);

	//Animation does not exist in this animator...
	if (iterator == m_animations.end())
	{
		return false;
	}
	
	m_currentAnim = m_animations[name];

	ResetTime();

	return true;
}

void RAnimator::Update()
{
	/*
		TODO: Need to be cleaned up
	*/

	//Needs a skeleton
	if (!m_bones.empty())
	{
		if (m_currentAnim)
		{
			//Do blending
			if (m_nextAnim)
			{
				//TODO: Make separate function
				//tickCurrent =  ...
				//tickNext = ...
				//check the blendtimer - where are we
			}
			//Do regular
			else
			{
				//TODO: Make separate function
				bool reachedEnd = false;

				double tick = m_currentAnim->GetTicksPerFrame() * Stats::Get().GetUpdateTime();
				m_currentFrameTime += tick;

				if (m_currentFrameTime >= m_currentAnim->GetDuraction())
				{
					if (m_currentAnim->IsLoopable())
					{
						m_currentFrameTime = fmod(m_currentFrameTime, m_currentAnim->GetDuraction());
					}
					else
					{
						m_currentAnim = m_defaultAnim;
						ResetTime();
						reachedEnd = true;
					}
				}

				if (!reachedEnd)
				{
					std::vector<sm::Matrix> modelMatrices;
					modelMatrices.resize(m_bones.size(), sm::Matrix::Identity);

					for (size_t i = 0; i < m_bones.size(); i++)
					{
						sm::Matrix localMatrix = m_currentAnim->GetMatrix(m_bones[i].name, m_currentFrameTime, m_bones[i].lastKeys, m_useInterpolation);

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
