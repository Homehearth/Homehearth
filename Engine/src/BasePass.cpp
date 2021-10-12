#include "EnginePCH.h"
#include "BasePass.h"

#include "PipelineManager.h"
#include "RMesh.h"

void BasePass::PreRender()
{	
	DC->IASetInputLayout(PM->m_defaultInputLayout.Get());
    
    DC->VSSetShader(PM->m_defaultVertexShader.Get(), nullptr, 0);
    DC->PSSetShader(PM->m_defaultPixelShader.Get(), nullptr, 0);

	DC->VSSetConstantBuffers(1, 1, CAMERA->m_viewConstantBuffer.GetAddressOf()); 
    
    DC->PSSetShaderResources(6, 1, PM->m_depthBufferSRV.GetAddressOf());   // DepthBuffer.

    DC->PSSetSamplers(0, 1, PM->m_linearSamplerState.GetAddressOf());
    DC->PSSetSamplers(1, 1, PM->m_pointSamplerState.GetAddressOf());
    
    DC->RSSetState(PM->m_rasterState.Get());

    DC->OMSetRenderTargets(1, PM->m_backBuffer.GetAddressOf(), nullptr);
    DC->OMSetDepthStencilState(PM->m_depthStencilStateEqualAndDisableDepthWrite.Get(), 0);
}

void BasePass::Render(Scene* pScene)
{
    pScene->Render();
}

void BasePass::PostRender()
{
	// Cleanup.
    ID3D11ShaderResourceView* nullSRV[] = { nullptr };
	DC->PSSetShaderResources(6, 1, nullSRV);
}

