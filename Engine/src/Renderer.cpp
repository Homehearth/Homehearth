#include "EnginePCH.h"
#include "Renderer.h"


Renderer::Renderer()
	: m_d3d11(nullptr)
	, m_camera(nullptr)
{
}

void Renderer::Initialize(Window* pWindow, Camera* pCamera)
{
	m_pipelineManager.Initialize(pWindow);
    m_camera = pCamera;
    m_d3d11 = &D3D11Core::Get();
	
    // DepthPass
    m_depthPass.SetName("DepthPass");
	m_depthPass.SetEnable(true);
    AddPass(&m_depthPass);
	
	// BasePass (Forward).
    m_basePass.SetName("BasePass");
    m_basePass.SetEnable(true);
    AddPass(&m_basePass);

    // Pass Camera to every pass.
	for(const auto & pass : m_passes)
	{
        pass->SetCamera(m_camera);
        pass->SetContext(m_d3d11->DeviceContext());
        pass->SetPipelineManager(&m_pipelineManager);
	} 

    LOG_INFO(GetInfoAboutPasses().c_str());
}

void Renderer::ClearFrame()
{
    // Clear the back buffer.
    const float m_clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_d3d11->DeviceContext()->ClearRenderTargetView(m_pipelineManager.m_backBufferTarget.Get(), m_clearColor);
    m_d3d11->DeviceContext()->ClearDepthStencilView(m_pipelineManager.m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderer::Render(Scene* pScene)
{
    if (pScene && m_camera)
    {
        if (!m_passes.empty())
        {
            UpdatePerFrame(pScene);
            for (const auto& pass : m_passes)
            {
                if (pass->IsEnabled())
                {
                    pass->PreRender();
                    pass->Render();
                    pass->PostRender();
                }
            }
        }
    }
}

void Renderer::AddPass(IRenderPass* pass)
{
    m_passes.emplace_back(pass);
}

void Renderer::UpdatePerFrame(Scene* pScene)
{
	// Give all passes the current Scene.
	for(const auto & pass : m_passes)
	{
        pass->SetScene(pScene);
	}

	// Update Camera.
    m_d3d11->DeviceContext()->UpdateSubresource(m_camera->m_viewConstantBuffer.Get(),
        0, nullptr, m_camera->GetCameraMatrixes(), 0, 0);
}

std::string Renderer::GetInfoAboutPasses()
{
    std::vector<std::string> activePasses;

	for(const auto & pass : m_passes)
	{
		if(pass->IsEnabled())
		{
            activePasses.emplace_back(pass->GetName());
		}
	}

    std::string info = std::to_string(activePasses.size()) + " of " + std::to_string(m_passes.size()) + " active rendering passes: ";
    for (const auto& pass : activePasses)
    {
        info += pass + " ";
    }
	
    return info;
}