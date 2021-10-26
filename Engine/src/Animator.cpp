#include "EnginePCH.h"
#include "Animator.h"

Animator::Animator()
{
	m_frameTime		= 0;
	m_currentAnim	= "";
	m_nextAnim		= "";
}

Animator::~Animator()
{
	m_bones.clear();
	m_finalMatrices.clear();
	m_animations.clear();
}

bool Animator::LoadModel(const std::string& filename)
{
	bool loaded = false;
	m_model = ResourceManager::Get().GetResource<RModel>(filename);

	if (m_model)
	{
		std::vector<bone_t> allBones = m_model->GetSkeleton();
		for (size_t i = 0; i < allBones.size(); i++)
		{
			bone_keyFrames_t bone;
			bone.name = allBones[i].name;
			bone.inverseBind = allBones[i].inverseBind;
			bone.parentIndex = allBones[i].parentIndex;
			m_bones.push_back(bone);
		}
		allBones.clear();

		m_finalMatrices.resize(m_bones.size(), sm::Matrix::Identity);

		//Creating the buffers needed for the GPU
		if (CreateBonesSB())
			loaded = true;
	}
	return loaded;
}

bool Animator::CreateBonesSB()
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

void Animator::UpdateStructureBuffer()
{
	D3D11_MAPPED_SUBRESOURCE submap;
	D3D11Core::Get().DeviceContext()->Map(m_bonesSB_Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &submap);
	memcpy(submap.pData, &m_finalMatrices[0], sizeof(sm::Matrix) * m_finalMatrices.size());
	D3D11Core::Get().DeviceContext()->Unmap(m_bonesSB_Buffer.Get(), 0);
}

void Animator::Bind() const
{
	D3D11Core::Get().DeviceContext()->VSSetShaderResources(T2D_BONESLOT, 1, m_bonesSB_RSV.GetAddressOf());
}

void Animator::Unbind() const
{
	ID3D11ShaderResourceView* nullSRV = nullptr;
	D3D11Core::Get().DeviceContext()->VSSetShaderResources(T2D_BONESLOT, 1, &nullSRV);
}

bool Animator::Create(const std::string& filename)
{
	/*
		Testing with hardcoded values for now...
	*/

	m_currentAnim = "Player_Idle.fbx";
	if (!LoadModel("Player_Skeleton.fbx"))
		return false;

	m_animations[m_currentAnim] = ResourceManager::Get().GetResource<RAnimation>(m_currentAnim);

	return true;
}

void Animator::Update()
{
	if (!m_bones.empty())
	{
		//double tickDT = m_animations[m_currentAnim]->GetTicksPerFrame() * 
		//m_frameTime +=  m_animations[m_currentAnim]->GetTicksPerFrame();
		//double nextFrameTime = 

		std::vector<sm::Matrix> modelMatrices;
		modelMatrices.resize(m_bones.size(), sm::Matrix::Identity);

		for (size_t i = 0; i < m_bones.size(); i++)
		{
			sm::Matrix localMatrix = m_animations[m_currentAnim]->GetMatrix(m_bones[i].name, 1.0, 0.02, m_bones[i].lastKeys, false);

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

void Animator::Render()
{
	//Unnecessary to the render if the model does not exist
	if (m_model)
	{
		Bind();
		m_model->Render();
		Unbind();
	}
}
