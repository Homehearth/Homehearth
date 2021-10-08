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
    m_basePass.SetEnable(true);
    m_depthPass.SetEnable(true);
    //AddPass(&m_depthPass);
    AddPass(&m_basePass);

    LOG_INFO("Number of rendering passes: %d", static_cast<int>(m_passes.size()));
}

void Renderer::ClearFrame()
{
    // Clear the back buffer.
    const float m_clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
    m_d3d11->DeviceContext()->ClearRenderTargetView(m_pipelineManager.m_renderTargetView.Get(), m_clearColor);
    m_d3d11->DeviceContext()->ClearDepthStencilView(m_pipelineManager.m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);  
}

void Renderer::Render(Scene* pScene)
{
    if (pScene)
    {
        if (!m_basePass.HasCamera())
        {
            m_basePass.SetCamera(pScene->GetCamera());
        }

        if (!m_passes.empty())
        {
            for (int i = 0; i < m_passes.size(); i++)
            {
                m_currentPass = i;
                IRenderPass* pass = m_passes[i];
                if (pass->IsEnabled())
                {
                    pass->PreRender(m_d3d11->DeviceContext(), &m_pipelineManager);
                    pass->Render(pScene);     // args? currently does nothing.
                    pass->PostRender(); // args? currently does nothing.
                }
            }
        }
    }
}

IRenderPass* Renderer::GetCurrentPass() const
{
    return m_passes[m_currentPass];
}

void Renderer::AddPass(IRenderPass* pass)
{
    m_passes.emplace_back(pass);
}