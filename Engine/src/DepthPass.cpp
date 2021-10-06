#include "EnginePCH.h"
#include "DepthPass.h"
#include "PipelineManager.h"
#include "RMesh.h"

void DepthPass::Initialize()
{
}

void DepthPass::PreRender(ID3D11DeviceContext* dc, PipelineManager* pm)
{
    // INPUT ASSEMBLY.
    {
        dc->IASetInputLayout(pm->m_defaultInputLayout.Get());
        dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    // SHADER STAGES.
    {
        dc->VSSetShader(pm->m_depthVertexShader.Get(), nullptr, 0);
        dc->PSSetShader(nullptr, nullptr, 0);
        dc->GSSetShader(nullptr, nullptr, 0);
        dc->HSSetShader(nullptr, nullptr, 0);
        dc->DSSetShader(nullptr, nullptr, 0);
        dc->CSSetShader(nullptr, nullptr, 0);
    }

    // CONSTANT BUFFERS.
    {
        dc->PSSetConstantBuffers(0, 0, nullptr);
        dc->VSSetConstantBuffers(0, 1, pm->m_defaultModelConstantBuffer.GetAddressOf());
        dc->VSSetConstantBuffers(1, 1, pm->m_defaultViewConstantBuffer.GetAddressOf());
    }

    // SHADER RESOURCES.
    {
        dc->PSSetShaderResources(0, 0, nullptr);
        dc->VSSetShaderResources(0, 0, nullptr);
        dc->PSSetSamplers(0, 0, nullptr);
    }

    // RASTERIZER.
    {
        dc->RSSetViewports(1, &pm->m_viewport);
        dc->RSSetState(nullptr);
    }

    // OUTPUT MERGER.
    {
        ID3D11RenderTargetView* nullRTV[] = { nullptr };		
        dc->OMSetRenderTargets(ARRAYSIZE(nullRTV), nullRTV, pm->m_depthStencilView.Get());
        dc->OMSetBlendState(nullptr, nullptr, 0);
        dc->OMSetDepthStencilState(pm->m_depthStencilStateLess.Get(), 0);
    }
}

void DepthPass::Render(Scene* pScene)
{
    // Render objects.
    pScene->Render();
}

void DepthPass::PostRender(ID3D11DeviceContext* dc, PipelineManager* pm)
{	
	// Cleanup.
    ID3D11RenderTargetView* nullRTV[] = { nullptr };
    dc->OMSetRenderTargets(_countof(nullRTV), nullRTV, nullptr);
}
