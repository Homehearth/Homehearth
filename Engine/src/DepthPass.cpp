#include "EnginePCH.h"
#include "DepthPass.h"
#include "PipelineManager.h"

void DepthPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
    // Set DepthBuffer.
    ID3D11RenderTargetView* nullRTV[] = { nullptr };
    DC->OMSetRenderTargets(0, nullRTV, PM->m_depthStencilView.Get());
    DC->OMSetDepthStencilState(PM->m_depthStencilStateLessOrEqualEnableDepthWrite.Get(), 0);

    DC->IASetInputLayout(PM->m_defaultInputLayout.Get());
    DC->PSSetSamplers(1, 1, PM->m_linearSamplerState.GetAddressOf());
    DC->RSSetViewports(1, &PM->m_viewport);

    DC->VSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());

    DC->VSSetShader(PM->m_depthPassVertexShader.Get(), nullptr, 0);
    DC->PSSetShader(PM->m_depthPassPixelShader.Get(), nullptr, 0);
}

void DepthPass::Render(Scene* pScene)
{
    PROFILE_FUNCTION();

	PM->SetCullBack(true, GetDeviceContext());

    pScene->RenderOpaque();

    PM->SetCullBack(false, GetDeviceContext());

    pScene->RenderTransparent();

	PM->SetCullBack(true, GetDeviceContext());
}

void DepthPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
    // Cleanup.
}
