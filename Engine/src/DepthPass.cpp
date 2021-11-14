#include "EnginePCH.h"
#include "DepthPass.h"
#include "PipelineManager.h"

void DepthPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
    // Set DepthBuffer.
    ID3D11RenderTargetView* nullRTV[] = { nullptr };
    DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DC->OMSetRenderTargets(ARRAYSIZE(nullRTV), nullRTV, PM->m_depthStencilView.Get());
    DC->OMSetDepthStencilState(PM->m_depthStencilStateLessEqual.Get(), 0);

    DC->RSSetViewports(1, &PM->m_viewport);
    DC->RSSetState(PM->m_rasterState.Get());
	DC->IASetInputLayout(PM->m_defaultInputLayout.Get());
    DC->VSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());

    DC->VSSetShader(PM->m_depthPassVertexShader.Get(), nullptr, 0);
    DC->PSSetShader(nullptr, nullptr, 0);
}

void DepthPass::Render(Scene* pScene)
{
    //SetCullBack(true);
	// render opaq.
	//SetCullBack(false);
	// render trans.
	//SetCullBack(true);
	pScene->Render();
}

void DepthPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
    // Cleanup.
    //ID3D11DepthStencilView* nullDSV = { nullptr };
    //DC->OMSetRenderTargets(0, nullptr, nullDSV);
}
