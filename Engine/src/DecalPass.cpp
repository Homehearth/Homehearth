#include "EnginePCH.h"
#include "DecalPass.h"

void DecalPass::CreateBuffer()
{
	if (m_buffer)
		m_buffer->Release();

	const size_t size = m_matrices.size() > 0 ? m_matrices.size() : 1;

	D3D11_BUFFER_DESC bDesc;
	bDesc.ByteWidth = sizeof(sm::Matrix) * size;
	bDesc.Usage = D3D11_USAGE_DEFAULT;
	bDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bDesc.StructureByteStride = sizeof(sm::Matrix);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &(m_matrices[0]);

	D3D11Core::Get().Device()->CreateBuffer(&bDesc, &data, &m_buffer);
}

DecalPass::DecalPass()
{
	m_buffer = nullptr;
}

DecalPass::~DecalPass()
{
	if (m_buffer)
		m_buffer->Release();
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
	DC->VSSetConstantBuffers(10, 1, &m_buffer);
}
