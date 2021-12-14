#include "EnginePCH.h"
#include "BasePass.h"

void BasePass::PreRender(Camera* pCam, ID3D11DeviceContext* pDeviceContext)
{	
    if (m_skyboxRef)
    {
        m_skyboxRef->Bind(pDeviceContext);
    }

    DC->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DC->IASetInputLayout(PM->m_defaultInputLayout.Get());
	
    DC->VSSetShader(PM->m_defaultVertexShader.Get(), nullptr, 0);
    DC->PSSetShader(PM->m_defaultPixelShader.Get(), nullptr, 0);
   
	DC->VSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf()); 
	DC->PSSetConstantBuffers(1, 1, pCam->m_viewConstantBuffer.GetAddressOf()); 
    
    //DC->PSSetShaderResources(0, 1, PM->m_depthBufferSRV.GetAddressOf());   // DepthBuffer.
   
    //Water blend map
    DC->PSSetShaderResources(22, 1, PM->m_SRV_ModdedTextureEffectBlendMap.GetAddressOf());


    DC->PSSetSamplers(0, 1, PM->m_pointSamplerState.GetAddressOf());
    DC->PSSetSamplers(1, 1, PM->m_linearSamplerState.GetAddressOf());
    DC->PSSetSamplers(2, 1, PM->m_anisotropicSamplerState.GetAddressOf());
    DC->PSSetSamplers(3, 1, PM->m_cubemapSamplerState.GetAddressOf());
    m_lights->Render(DC);
    
    DC->RSSetViewports(1, &PM->m_viewport);
    DC->RSSetState(PM->m_rasterState.Get());

    ID3D11RenderTargetView* renderTargets[2];
    renderTargets[0] = PM->m_backBuffer.Get();
    renderTargets[1] = PM->m_bloomTargetView.Get();

    DC->OMSetRenderTargets(2, renderTargets, PM->m_depthStencilView.Get());
    DC->OMSetDepthStencilState(PM->m_depthStencilStateLessEqual.Get(), 0);

    if(m_pShadowPass)
        m_pShadowPass->PostRender(DC);

}

void BasePass::Render(Scene* pScene)
{
    pScene->Render();
}

void BasePass::PostRender(ID3D11DeviceContext* pDeviceContext)
{
	// Cleanup.
	//ID3D11ShaderResourceView* nullSRV[] = { nullptr };
	//DC->PSSetShaderResources(0, 1, nullSRV);
    ID3D11RenderTargetView* nullRender[2];
    nullRender[0] = nullptr;
    nullRender[1] = nullptr;
    DC->OMSetRenderTargets(2, nullRender, nullptr);

    ID3D11Buffer* nullBuffer = nullptr;
    DC->PSSetConstantBuffers(1, 1, &nullBuffer);
    DC->VSSetConstantBuffers(1, 1, &nullBuffer);

    ID3D11ShaderResourceView* kill = nullptr;
    DC->PSSetShaderResources(22, 1, &kill);
}
