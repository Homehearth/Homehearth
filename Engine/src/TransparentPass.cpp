#include "EnginePCH.h"
#include "TransparentPass.h"

void TransparentPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
    // Transparent pipeline.
	DC->OMSetRenderTargets(1, PM->m_backBuffer.GetAddressOf(), nullptr);
	DC->OMSetDepthStencilState(PM->m_depthStencilStateLessOrEqual.Get(), 0);
    DC->IASetInputLayout(PM->m_defaultInputLayout.Get());
    
    // Blend mode for alpha blending.
    constexpr float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    DC->OMSetBlendState(PM->m_alphaBlending.Get(), blendFactor, 0xffffffff);

    DC->VSSetShader(PM->m_defaultVertexShader.Get(), nullptr, 0);
    DC->PSSetShader(PM->m_defaultPixelShader.Get(), nullptr, 0);

    DC->VSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());
    DC->PSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());

    DC->RSSetViewports(1, &PM->m_viewport);
    DC->RSSetState(PM->m_rasterStateNoCulling.Get());

    DC->PSSetSamplers(0, 1, PM->m_pointSamplerState.GetAddressOf());
    DC->PSSetSamplers(1, 1, PM->m_linearSamplerState.GetAddressOf());
    DC->PSSetSamplers(2, 1, PM->m_anisotropicSamplerState.GetAddressOf());
    DC->PSSetSamplers(3, 1, PM->m_cubemapSamplerState.GetAddressOf());
    //m_lights->Render(DC);
    PM->SetCullBack(false, DC);
}

void TransparentPass::Render(Scene* pScene)
{
    pScene->RenderTransparent();
}

void TransparentPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
    PM->SetCullBack(true, DC);
}
