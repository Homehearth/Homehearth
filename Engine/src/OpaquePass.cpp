#include "EnginePCH.h"
#include "OpaquePass.h"

void OpaquePass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
    if(m_skyBoxRef)
    {
        m_skyBoxRef->Bind(pDeviceContext);
    }

    ID3D11RenderTargetView* renderTargets[2];
    renderTargets[0] = PM->m_backBuffer.Get();
    renderTargets[1] = PM->m_bloomTargetView.Get();

    DC->OMSetRenderTargets(2, renderTargets, PM->m_depth.dsv.Get());
    DC->OMSetDepthStencilState(PM->m_depthStencilStateLessOrEqual.Get(), 0);
    DC->IASetInputLayout(PM->m_defaultInputLayout.Get());

    DC->VSSetShader(PM->m_defaultVertexShader.Get(), nullptr, 0);
    DC->PSSetShader(PM->m_opaquePixelShader.Get(), nullptr, 0);

    DC->VSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());
    DC->PSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());
    
    DC->RSSetViewports(1, &PM->m_viewport);

    DC->PSSetSamplers(0, 1, PM->m_pointSamplerState.GetAddressOf());
    DC->PSSetSamplers(1, 1, PM->m_linearSamplerState.GetAddressOf());
    DC->PSSetSamplers(2, 1, PM->m_anisotropicSamplerState.GetAddressOf());
    DC->PSSetSamplers(3, 1, PM->m_cubemapSamplerState.GetAddressOf());

	m_lights->Render(DC);

    DC->PSSetShaderResources(25, 1, PM->opaq_LightIndexList.srv.GetAddressOf());
    DC->PSSetShaderResources(23, 1, PM->opaq_LightGrid.srv.GetAddressOf());

    if (m_shadowPassRef)
        m_shadowPassRef->PostRender(DC);

    PM->SetCullBack(true, DC);
}

void OpaquePass::Render(Scene* pScene)
{
    pScene->RenderOpaqueThreaded();
}

void OpaquePass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
    ID3D11RenderTargetView* nullRender[2];
    nullRender[0] = nullptr;
    nullRender[1] = nullptr;
    DC->OMSetRenderTargets(2, nullRender, nullptr);

    ID3D11ShaderResourceView* nullSRV[] = { nullptr };
    DC->PSSetShaderResources(25, 1, nullSRV);
    DC->PSSetShaderResources(23, 1, nullSRV);
}
