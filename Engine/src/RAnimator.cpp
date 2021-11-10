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

bool RAnimator::Create(const std::string& filename)
{
	std::ifstream readfile(ANIMATORPATH + filename);
	std::string line;

	/*
		Load from a customfile later.
		Loads in all the animations
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
				m_currentAnim = animation;
			}
		}
		else if (keyword == "currentAnim")
		{
			std::string key;
			ss >> key;

			if (m_animations.find(key) != m_animations.end())
			{
				m_currentAnim = m_animations[key];
			}
		}
	}

	readfile.close();

	return true;
}

void RAnimator::Update()
{
	if (!m_bones.empty() && m_currentAnim)
	{
		double tick = m_currentAnim->GetTicksPerFrame() * Stats::Get().GetUpdateTime();
		m_currentFrameTime = fmod(m_currentFrameTime + tick, m_currentAnim->GetDuraction());
		
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
