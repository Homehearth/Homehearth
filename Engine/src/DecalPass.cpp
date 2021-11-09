#include "EnginePCH.h"
#include "DecalPass.h"

void DecalPass::CreateBuffer()
{
	if (m_buffer)
	{
		m_buffer->Release();
		m_buffer = nullptr;
	}

	const size_t size = m_matrices.size() > 0 ? m_matrices.size() : 1;

	D3D11_BUFFER_DESC bDesc;
	bDesc.ByteWidth = sizeof(sm::Matrix) * size;
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

	sm::Vector4 info = { (float)m_matrices.size(), 0.0f, 0.0f, 0.0f };

	m_infoBuffer.SetData(D3D11Core::Get().DeviceContext(), info);
}

void DecalPass::Create()
{
	m_infoBuffer.Create(D3D11Core::Get().Device());
}

DecalPass::DecalPass()
{
	m_buffer = nullptr;
	m_shaderView = nullptr;
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
	//if (pCam)
	//{
	//	DC->VSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());
	//}

	//DC->OMSetRenderTargets(1, PM->m_backBuffer.GetAddressOf(), PM->m_depthStencilView.Get());
	//DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//DC->RSSetViewports(1, &PM->m_viewport);
	//DC->RSSetState(PM->m_rasterState.Get());
	//DC->OMSetDepthStencilState(PM->m_depthStencilStateLessEqual.Get(), 0);
}

void DecalPass::Render(Scene* pScene)
{
	m_matrices.clear();
	pScene->ForEachComponent<comp::Decal>([&](comp::Decal& d) {

		// Decrease lifespan.
		d.lifespan -= Stats::GetDeltaTime();
		if (d.lifespan > 0)
		{
			m_matrices.push_back(d.viewPoint);
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

	DC->PSSetShaderResources(15, 1, &m_shaderView);
	DC->PSSetConstantBuffers(10, 1, &buff);
}
