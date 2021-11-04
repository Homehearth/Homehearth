#include "EnginePCH.h"
#include "AnimationPass.h"

/*
    Uses other inputlayout and vertexshader than basepass
*/
void AnimationPass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{
    DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DC->IASetInputLayout(PM->m_animationInputLayout.Get());

    DC->VSSetShader(PM->m_animationVertexShader.Get(), nullptr, 0);
    DC->PSSetShader(PM->m_defaultPixelShader.Get(), nullptr, 0);

    DC->VSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf());

    //DC->PSSetShaderResources(0, 1, PM->m_depthBufferSRV.GetAddressOf());   // DepthBuffer.

    DC->PSSetSamplers(0, 1, PM->m_anisotropicSamplerState.GetAddressOf());
    DC->PSSetSamplers(1, 1, PM->m_pointSamplerState.GetAddressOf());
    m_lights->Render(DC);

    DC->RSSetViewports(1, &PM->m_viewport);
    DC->RSSetState(PM->m_rasterState.Get());

    DC->OMSetRenderTargets(1, PM->m_backBuffer.GetAddressOf(), PM->m_depthStencilView.Get());
    DC->OMSetDepthStencilState(PM->m_depthStencilStateLessEqual.Get(), 0);
}

void AnimationPass::Render(Scene* pScene)
{
    pScene->RenderAnimation();
}

void AnimationPass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
	// Cleanup.
	//ID3D11ShaderResourceView* nullSRV[] = { nullptr };
	//DC->PSSetShaderResources(0, 1, nullSRV);
}
