#include "EnginePCH.h"
#include "DepthPass.h"
#include "PipelineManager.h"

void DepthPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
    // Set DepthBuffer.
    ID3D11RenderTargetView* nullRTV[] = { nullptr };
    DC->OMSetRenderTargets(0, nullRTV, PM->m_depth.dsv.Get());
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

    pScene->GetRegistry()->view<comp::Animator>().each([&](comp::Animator& anim)
        {
            if (anim.animator && anim.updating)
            {
                anim.animator->Update();
            }
        });

	PM->SetCullBack(true, GetDeviceContext());

    pScene->RenderOpaque(PM);

    PM->SetCullBack(false, GetDeviceContext());
    D3D11Core::Get().DeviceContext()->VSSetShader(PM->m_depthPassVertexShader.Get(), nullptr, 0);
    D3D11Core::Get().DeviceContext()->IASetInputLayout(PM->m_defaultInputLayout.Get());
    pScene->RenderTransparent();

	PM->SetCullBack(true, GetDeviceContext());
}

void DepthPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
    // Cleanup.
}