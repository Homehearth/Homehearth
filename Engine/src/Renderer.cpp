#include "EnginePCH.h"
#include "Renderer.h"


Renderer::Renderer()
	: m_d3d11(nullptr), m_camera(nullptr)
{
}

void Renderer::Initialize(Window* pWindow, Camera* pCamera)
{
	m_pipelineManager.Initialize(pWindow);
    m_camera = pCamera;

    m_d3d11 = &D3D11Core::Get();
    //AddPass(&m_depthPass);
    AddPass(&m_basePass);

    for(auto & pass : m_passes)
    {
        pass->Initialize(m_camera, m_d3d11->DeviceContext(), &m_pipelineManager);
        pass->SetEnable(true);
    }

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
    if (pScene && m_camera)
    {
        if (!m_passes.empty())
        {
            UpdatePerFrame(); // once per frame.
            for (const auto& pass : m_passes)
            {
                if (pass->IsEnabled())
                {
                    pass->PreRender();
                    pass->Render(pScene);
                    pass->PostRender(); 
                }
            }

            pScene->ReadyForSwap(); 
        }
    }
}

void Renderer::UpdatePerFrame()
{
	// Update Camera constant buffer.
    m_d3d11->DeviceContext()->UpdateSubresource(m_camera->m_viewConstantBuffer.Get(), 0, nullptr, m_camera->GetCameraMatrixes(), 0, 0);
}


void Renderer::AddPass(IRenderPass* pass)
{
    m_passes.emplace_back(pass);
}