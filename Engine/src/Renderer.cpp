#include "EnginePCH.h"
#include "Renderer.h"


Renderer::Renderer()
	: m_d3d11(nullptr)
{
}

void Renderer::Initialize(Window* pWindow)
{
	m_pipelineManager.Initialize(pWindow);
	
    m_d3d11 = &D3D11Core::Get();
    AddPass(&m_basePass);
}

void Renderer::ClearFrame()
{
    // Clear the back buffer.
    const float m_clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_d3d11->DeviceContext()->ClearRenderTargetView(m_pipelineManager.m_renderTargetView.Get(), m_clearColor);
}

void Renderer::Render()
{
    if (!m_passes.empty())
    {
        for (const auto& pass : m_passes)
        {
            if (pass->IsEnabled())
            {
                pass->PreRender(m_d3d11->DeviceContext(), &m_pipelineManager);
                pass->Render();     // args?
                pass->PostRender(); // args?
            }
        }
    }
}

void Renderer::AddPass(IRenderPass* pass)
{
    m_passes.emplace_back(pass);
}

void Renderer::SetPipelineState()
{
#define CONTEXT D3D11Core::Get().DeviceContext()
	
    CONTEXT->PSSetSamplers(0, 1, m_pipelineManager.m_linearSamplerState.GetAddressOf());
    CONTEXT->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    CONTEXT->OMSetRenderTargets(1, m_pipelineManager.m_renderTargetView.GetAddressOf(), m_pipelineManager.m_depthStencilView.Get());
    CONTEXT->VSSetConstantBuffers(0, 1, m_pipelineManager.m_defaultConstantBuffer.GetAddressOf());
    CONTEXT->IASetInputLayout(m_pipelineManager.m_defaultInputLayout.Get());
    CONTEXT->VSSetShader(m_pipelineManager.m_defaultVertexShader.Get(), nullptr, NULL);
    CONTEXT->PSSetShader(m_pipelineManager.m_defaultPixelShader.Get(), nullptr, NULL);
}
