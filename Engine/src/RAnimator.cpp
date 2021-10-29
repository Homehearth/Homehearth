#include "EnginePCH.h"
#include "RAnimator.h"

RAnimator::RAnimator()
{
	m_frameTime		= 0;
	m_currentAnim	= "";
	m_nextAnim		= "";
}

RAnimator::~RAnimator()
{
	m_bones.clear();
	m_finalMatrices.clear();
	m_animations.clear();
}

//bool RAnimator::LoadSkeleton(const std::shared_ptr<RModel>& model)
bool RAnimator::LoadSkeleton(const std::vector<bone_t>& skeleton)
{
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
	/*
		Testing with hardcoded values for now...
	*/

	m_currentAnim = "Player_Idle.fbx";

	m_animations[m_currentAnim] = ResourceManager::Get().GetResource<RAnimation>(m_currentAnim);

	return true;
}

void RAnimator::Update()
{
	if (!m_bones.empty())
	{
		double tickDT = m_animations[m_currentAnim]->GetTicksPerFrame() * Stats::GetDeltaTime();

		m_frameTime = fmod(m_frameTime + tickDT, m_animations[m_currentAnim]->GetDuraction());

		double nextFrameTime = m_frameTime + tickDT;

		std::vector<sm::Matrix> modelMatrices;
		modelMatrices.resize(m_bones.size(), sm::Matrix::Identity);

		for (size_t i = 0; i < m_bones.size(); i++)
		{
			sm::Matrix localMatrix = m_animations[m_currentAnim]->GetMatrix(m_bones[i].name, m_frameTime, nextFrameTime, m_bones[i].lastKeys, true);

			if (m_bones[i].parentIndex == -1)
				modelMatrices[i] = localMatrix;
			else
				modelMatrices[i] = localMatrix * modelMatrices[m_bones[i].parentIndex];
	
			m_finalMatrices[i] = m_bones[i].inverseBind * modelMatrices[i];
		}

		modelMatrices.clear();

		UpdateStructureBuffer();
	}
}

void RAnimator::Bind() const
{
	D3D11Core::Get().DeviceContext()->VSSetShaderResources(T2D_BONESLOT, 1, m_bonesSB_RSV.GetAddressOf());
}

void RAnimator::Unbind() const
{
	ID3D11ShaderResourceView* nullSRV = nullptr;
	D3D11Core::Get().DeviceContext()->VSSetShaderResources(T2D_BONESLOT, 1, &nullSRV);
}
