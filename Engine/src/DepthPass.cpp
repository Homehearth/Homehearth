#include "EnginePCH.h"
#include "DepthPass.h"
#include "PipelineManager.h"
#include "RMesh.h"

void DepthPass::Initialize()
{
    // Initialize Pass.
    // accept scene.
    // accept materials.
    // accept ...
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
        dc->VSSetShader(nullptr, nullptr, 0);
        dc->PSSetShader(nullptr, nullptr, 0);
        dc->GSSetShader(nullptr, nullptr, 0);
        dc->HSSetShader(nullptr, nullptr, 0);
        dc->DSSetShader(nullptr, nullptr, 0);
        dc->CSSetShader(nullptr, nullptr, 0);
    }

    // CONSTANT BUFFERS.
    {
        dc->PSSetConstantBuffers(0, 0, nullptr);
        dc->VSSetConstantBuffers(0, 0, nullptr);
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
        dc->OMSetRenderTargets(0, nullptr, nullptr);
        dc->OMSetBlendState(nullptr, nullptr, 0); 
        dc->OMSetDepthStencilState(nullptr, 0);
    }
}

void DepthPass::Render()
{
    // Render objects.
}

void DepthPass::PostRender()
{
    // return rendertarget for next pass?
}