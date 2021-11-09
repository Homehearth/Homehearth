#include "EnginePCH.h"
#include "DecalPass.h"

void DecalPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
	if (pCam)
	{
		DC->VSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());
	}

	DC->OMSetRenderTargets(1, PM->m_backBuffer.GetAddressOf(), PM->m_depthStencilView.Get());
	DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DC->RSSetViewports(1, &PM->m_viewport);
	DC->RSSetState(PM->m_rasterState.Get());
	DC->OMSetDepthStencilState(PM->m_depthStencilStateLessEqual.Get(), 0);
}

void DecalPass::Render(Scene* pScene)
{
	pScene->RenderDecals();
}

void DecalPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{

}
