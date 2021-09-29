#include "EnginePCH.h"
#include "BasePass.h"

#include "PipelineManager.h"
#include "RMesh.h"

void BasePass::Initialize()
{
	// Initialize Pass.
	// accept scene.
	// accept materials.
	// accept ...
}

void BasePass::PreRender(ID3D11DeviceContext* dc, PipelineManager* pm)
{
    // INPUT ASSEMBLY.
    {
        dc->IASetInputLayout(pm->m_defaultInputLayout.Get());
        dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }

    // SHADER STAGES.
    {
        dc->VSSetShader(pm->m_defaultVertexShader.Get(), nullptr, 0);
        dc->PSSetShader(pm->m_defaultPixelShader.Get(), nullptr, 0);
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
        dc->PSSetSamplers(0, 1, pm->m_linearSamplerState.GetAddressOf());
    }

    // RASTERIZER.
    {
        dc->RSSetViewports(1, &pm->m_viewport);
        dc->RSSetState(pm->m_rasterStateNoCulling.Get());
    }

    // OUTPUT MERGER.
    {
        dc->OMSetRenderTargets(1, pm->m_renderTargetView.GetAddressOf(), pm->m_depthStencilView.Get());
        // TODO set appropriate blendState
        //dc->OMSetBlendState(nullptr, nullptr, 0); 
        dc->OMSetDepthStencilState(pm->m_depthStencilState.Get(), 1);
    }
}

void BasePass::Render()
{
	// Render objects.
}

void BasePass::PostRender()
{
	// return rendertarget for next pass?
}