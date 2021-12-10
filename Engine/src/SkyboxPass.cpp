#include "EnginePCH.h"
#include "SkyboxPass.h"

void SkyboxPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
    DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DC->IASetInputLayout(PM->m_skyboxInputLayout.Get());

    DC->VSSetShader(PM->m_skyboxVertexShader.Get(), nullptr, 0);
    DC->PSSetShader(PM->m_skyboxPixelShader.Get(), nullptr, 0);

    DC->VSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());

    DC->PSSetSamplers(0, 1, PM->m_linearSamplerState.GetAddressOf());
    DC->PSSetSamplers(1, 1, PM->m_pointSamplerState.GetAddressOf());
    DC->PSSetSamplers(3, 1, PM->m_cubemapSamplerState.GetAddressOf());

    DC->RSSetState(PM->m_rasterStateNoCulling.Get());

    DC->OMSetDepthStencilState(PM->m_depthStencilStateLessOrEqual.Get(), 0);
}

void SkyboxPass::Render(Scene* pScene)
{
    PROFILE_FUNCTION();

    pScene->RenderSkybox();
}

void SkyboxPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
}
