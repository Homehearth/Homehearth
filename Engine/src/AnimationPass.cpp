#include "EnginePCH.h"
#include "AnimationPass.h"

/*
    Uses other inputlayout and vertexshader than basepass
*/
void AnimationPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
    if (m_skyboxRef)
    {
        m_skyboxRef->Bind(pDeviceContext);
    }

    DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DC->IASetInputLayout(PM->m_animationInputLayout.Get());

    DC->VSSetShader(PM->m_animationVertexShader.Get(), nullptr, 0);
    DC->PSSetShader(PM->m_opaquePixelShader.Get(), nullptr, 0);

    DC->VSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());
    DC->PSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());

    //DC->PSSetShaderResources(0, 1, PM->m_depthBufferSRV.GetAddressOf());   // DepthBuffer.

    DC->PSSetSamplers(0, 1, PM->m_pointSamplerState.GetAddressOf());
    DC->PSSetSamplers(1, 1, PM->m_linearSamplerState.GetAddressOf());
    DC->PSSetSamplers(2, 1, PM->m_anisotropicSamplerState.GetAddressOf());
    m_lights->Render(DC);

    DC->RSSetViewports(1, &PM->m_viewport);

    ID3D11RenderTargetView* renderTargets[2];
    renderTargets[0] = PM->m_backBuffer.Get();
    renderTargets[1] = PM->m_bloomTargetView.Get();

    DC->PSSetShaderResources(25, 1, PM->opaq_LightIndexList.srv.GetAddressOf());
    DC->PSSetShaderResources(23, 1, PM->opaq_LightGrid.srv.GetAddressOf());

    DC->OMSetRenderTargets(2, renderTargets, PM->m_depth.dsv.Get());
    DC->OMSetDepthStencilState(PM->m_depthStencilStateLessOrEqual.Get(), 0);
    PM->SetCullBack(true, DC);

    if (m_pShadowPass)
        m_pShadowPass->PostRender(DC);
}

void AnimationPass::Render(Scene* pScene)
{
    PROFILE_FUNCTION();

    pScene->RenderAnimation();
}

void AnimationPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
    // Cleanup.
    //ID3D11ShaderResourceView* nullSRV[] = { nullptr };
    //DC->PSSetShaderResources(0, 1, nullSRV);
    ID3D11ShaderResourceView* nullSRV[] = { nullptr };
    DC->PSSetShaderResources(25, 1, nullSRV);
    DC->PSSetShaderResources(23, 1, nullSRV);
}
