#include "EnginePCH.h"
#include "DepthPass.h"
#include "PipelineManager.h"

void DepthPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
    // Setup.
    ID3D11RenderTargetView* nullRTV[] = { nullptr };

    DC->OMSetRenderTargets(ARRAYSIZE(nullRTV), nullRTV, PM->m_depthStencilView.Get());
    DC->OMSetDepthStencilState(PM->m_depthStencilStateLessEqual.Get(), 0);

    DC->RSSetViewports(1, &PM->m_viewport);
	DC->IASetInputLayout(PM->m_defaultInputLayout.Get());
    DC->VSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());  // Camera.

    DC->VSSetShader(PM->m_depthPassVertexShader.Get(), nullptr, 0);
    DC->PSSetShader(nullptr, nullptr, 0);
}

void DepthPass::Render(Scene* pScene)
{
    PM->SetCullBack(true);

    // Render opaque.
	pScene->Render(); // Opaque Mesh

    PM->SetCullBack(false);

	// Render trans.
    pScene->Render(); // Alpha Mesh
}

void DepthPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
    // Cleanup.
    PM->SetCullBack(true);
}
