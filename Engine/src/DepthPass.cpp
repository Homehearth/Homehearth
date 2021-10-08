#include "EnginePCH.h"
#include "DepthPass.h"
#include "PipelineManager.h"
#include "RMesh.h"

void DepthPass::PreRender()
{
    ID3D11DeviceContext* dc = GetDeviceContext();
    PipelineManager* pm = GetPipelineManager();

	dc->IASetInputLayout(pm->m_defaultInputLayout.Get());
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    dc->VSSetShader(pm->m_depthPassVertexShader.Get(), nullptr, 0);
    dc->PSSetShader(nullptr, nullptr, 0);
	
    dc->VSSetConstantBuffers(1, 1, GetCamera()->m_viewConstantBuffer.GetAddressOf());
 
    ID3D11RenderTargetView* nullRTV[] = { nullptr };
    dc->OMSetRenderTargets(ARRAYSIZE(nullRTV), nullRTV, pm->m_depthStencilView.Get());
    dc->OMSetDepthStencilState(pm->m_depthStencilStateLess.Get(), 0);

}

void DepthPass::Render(Scene* pScene)
{
   pScene->Render(); 
}

void DepthPass::PostRender()
{

}