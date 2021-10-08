#include "EnginePCH.h"
#include "BasePass.h"

#include "PipelineManager.h"
#include "RMesh.h"

void BasePass::PreRender()
{
    ID3D11DeviceContext* dc = GetDeviceContext();
    PipelineManager* pm = GetPipelineManager();
	
	dc->IASetInputLayout(pm->m_defaultInputLayout.Get());
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    dc->VSSetShader(pm->m_defaultVertexShader.Get(), nullptr, 0);
    dc->PSSetShader(pm->m_defaultPixelShader.Get(), nullptr, 0);

	dc->VSSetConstantBuffers(1, 1, GetCamera()->m_viewConstantBuffer.GetAddressOf());
    dc->PSSetConstantBuffers(0, 0, nullptr);        
    
    //dc->PSSetShaderResources(10, 1, pm->m_depthBufferSRV.GetAddressOf());   // DepthBuffer currently deactivated.
	
    dc->PSSetSamplers(0, 1, pm->m_linearSamplerState.GetAddressOf());
    dc->PSSetSamplers(1, 1, pm->m_pointSamplerState.GetAddressOf());
    
    dc->RSSetState(pm->m_rasterState.Get());
    
    dc->OMSetRenderTargets(1, pm->m_renderTargetView.GetAddressOf(), pm->m_depthStencilView.Get());
    dc->OMSetDepthStencilState(pm->m_depthStencilStateLess.Get(), 0);
}

void BasePass::Render(Scene* pScene)
{
    pScene->Render();
}

void BasePass::PostRender()
{
	// Cleanup.
    ID3D11ShaderResourceView* nullSRV[] = { nullptr };
    GetDeviceContext()->PSSetShaderResources(10, ARRAYSIZE(nullSRV), nullSRV);
}

