#include "EnginePCH.h"
#include "OpaquePass.h"

void OpaquePass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
    // Opaque pipeline
    DC->OMSetRenderTargets(1, PM->m_backBuffer.GetAddressOf(), nullptr);
    //DC->OMSetDepthStencilState(PM->m_depthStencilStateGreater.Get(), 0);
    DC->IASetInputLayout(PM->m_defaultInputLayout.Get());

    DC->VSSetShader(PM->m_defaultVertexShader.Get(), nullptr, 0);
    DC->PSSetShader(PM->m_defaultPixelShader.Get(), nullptr, 0);

    DC->VSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());
    DC->PSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());

    DC->RSSetViewports(1, &PM->m_viewport);

    DC->PSSetSamplers(0, 1, PM->m_pointSamplerState.GetAddressOf());
    DC->PSSetSamplers(1, 1, PM->m_linearSamplerState.GetAddressOf());
    DC->PSSetSamplers(2, 1, PM->m_anisotropicSamplerState.GetAddressOf());
    DC->PSSetSamplers(3, 1, PM->m_cubemapSamplerState.GetAddressOf());
    m_lights->Render(DC);
    PM->SetCullBack(true, DC);
}

void OpaquePass::Render(Scene* pScene)
{
    pScene->RenderOpaque();
}

void OpaquePass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
}
