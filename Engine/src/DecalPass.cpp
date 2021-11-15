#include "EnginePCH.h"
#include "DecalPass.h"

void DecalPass::CreateBuffer()
{
	if (m_buffer)
	{
		m_buffer->Release();
		m_buffer = nullptr;
	}
	if (m_shaderView)
	{
		m_shaderView->Release();
		m_shaderView = nullptr;
	}

	const size_t size = m_matrices.size() > 0 ? m_matrices.size() : 1;

	D3D11_BUFFER_DESC bDesc;
	bDesc.ByteWidth = sizeof(sm::Matrix) * (UINT)size;
	bDesc.Usage = D3D11_USAGE_DEFAULT;
	bDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bDesc.StructureByteStride = sizeof(sm::Matrix);

	D3D11_SUBRESOURCE_DATA data;
	// Dont make data if matrices is below 0.
	if (m_matrices.size() > 0)
	{
		data.pSysMem = &(m_matrices[0]);
		HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&bDesc, &data, &m_buffer);

		if (SUCCEEDED(hr))
			D3D11Core::Get().Device()->CreateShaderResourceView(m_buffer, NULL, &m_shaderView);
	}
	
	m_decalInfoBuffer.info = { (float)m_matrices.size(), 0.0f, 0.0f, 0.0f };

	m_infoBuffer.SetData(D3D11Core::Get().DeviceContext(), m_decalInfoBuffer);
}

void DecalPass::Create()
{
	m_infoBuffer.Create(D3D11Core::Get().Device());
	tempTexture = ResourceManager::Get().GetResource<RTexture>("demoBloodsplat.png");
	tempAlphaTexture = ResourceManager::Get().GetResource<RTexture>("demoBloodsplatAlpha.png");
}

DecalPass::DecalPass()
{
	m_buffer = nullptr;
	m_shaderView = nullptr;

	m_decalInfoBuffer.projection = dx::XMMatrixPerspectiveFovLH(3.1415F * 0.15F, 1.777777777777778F, 7.5f, 10.0f);
}

DecalPass::~DecalPass()
{
	if (m_buffer)
		m_buffer->Release();
	if (m_shaderView)
		m_shaderView->Release();
}

void DecalPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
	if (tempTexture)
	{
		DC->PSSetShaderResources(12, 1, &tempTexture.get()->GetShaderView());
	}

	if (tempAlphaTexture)
	{
		DC->PSSetShaderResources(13, 1, &tempAlphaTexture.get()->GetShaderView());
	}
}

void DecalPass::Render(Scene* pScene)
{
	m_matrices.clear();
	pScene->ForEachComponent<comp::Decal>([&](Entity e, comp::Decal& d) {

		// Decrease lifespan.
		d.lifespan -= Stats::Get().GetFrameTime();
		if (d.lifespan > 0)
		{
			m_matrices.push_back(d.viewPoint);
		}

		/*
			Remove the component when not needed anymore.
		*/
		if (d.lifespan <= 0)
		{
			e.Destroy();
		}
		});
	
	this->CreateBuffer();
}

void DecalPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
	ID3D11Buffer* buff =
	{
		m_infoBuffer.GetBuffer()
	};

	DC->PSSetShaderResources(16, 1, &m_shaderView);
	DC->PSSetConstantBuffers(10, 1, &buff);
}
