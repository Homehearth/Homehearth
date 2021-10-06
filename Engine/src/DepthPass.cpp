#include "EnginePCH.h"
#include "DepthPass.h"
#include "PipelineManager.h"
#include "RMesh.h"

void DepthPass::Initialize()
{
}

void DepthPass::PreRender()
{
    ClearPipelineSettings();
	
    auto dc = GetContext();
    auto pm = GetPipelineManager();

    // INPUT ASSEMBLY.
    {
        dc->IASetInputLayout(pm->m_defaultInputLayout.Get());
        dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    // SHADER STAGES.
    {
        dc->VSSetShader(pm->m_depthVertexShader.Get(), nullptr, 0);
    }

    // CONSTANT BUFFERS.
    {
        dc->VSSetConstantBuffers(0, 1, GetCamera()->m_viewConstantBuffer.GetAddressOf());
    }

    // SHADER RESOURCES.
    {
    }

    // RASTERIZER.
    {
        dc->RSSetViewports(1, &pm->m_viewport);
    }

    // OUTPUT MERGER.
    {
        ID3D11RenderTargetView* nullRTV[] = { nullptr };		
        dc->OMSetRenderTargets(ARRAYSIZE(nullRTV), nullRTV, pm->m_depthStencilView.Get());
        dc->OMSetDepthStencilState(pm->m_greaterDSS.Get(), 0);
    }
}

void DepthPass::Render()
{
    ID3D11Buffer* buffers[] = { GetScene()->GetRenderableBuffer().GetBuffer() };
	
    // System that renders Renderable component.
    GetContext()->VSSetConstantBuffers(0, 1, buffers);
	
    auto& renderableCopies = GetScene()->GetRenderableCopies();
    for (auto& it : renderableCopies[1])
    {
        GetScene()->GetRenderableBuffer().SetData(D3D11Core::Get().DeviceContext(), it.data);
        it.mesh->Render();
    }

    renderableCopies.ReadyForSwap();
}

void DepthPass::PostRender()
{	

    
}
