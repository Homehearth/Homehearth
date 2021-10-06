#include "EnginePCH.h"
#include "BasePass.h"

#include "PipelineManager.h"
#include "RMesh.h"

void BasePass::Initialize()
{
}

void BasePass::PreRender()
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
        dc->VSSetShader(pm->m_defaultVertexShader.Get(), nullptr, 0);
        dc->PSSetShader(pm->m_defaultPixelShader.Get(), nullptr, 0);
    }

    // CONSTANT BUFFERS.
    {
        dc->VSSetConstantBuffers(1, 1, GetCamera()->m_viewConstantBuffer.GetAddressOf());
    }

    // SHADER RESOURCES.
    {
        dc->PSSetShaderResources(10, 1, pm->m_depthBufferSRV.GetAddressOf());	// DepthBuffer.
        dc->PSSetSamplers(0, 1, pm->m_linearSamplerState.GetAddressOf());
        dc->PSSetSamplers(1, 1, pm->m_pointSamplerState.GetAddressOf());
    }

    // RASTERIZER.
    {
        dc->RSSetViewports(1, &pm->m_viewport);
        dc->RSSetState(pm->m_rasterStateNoCulling.Get());
    }

    // OUTPUT MERGER.
    {
        dc->OMSetRenderTargets(1, pm->m_backBufferTarget.GetAddressOf(), nullptr);
        dc->OMSetDepthStencilState(pm->m_lessDSS.Get(), 0);
        dc->OMSetBlendState(pm->m_blendStatepOpaque.Get(), nullptr, 0xFFFFFFFF); 
    }
}

void BasePass::Render()
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

void BasePass::PostRender()
{

    
}
