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

    // DepthPass
    m_depthPass.SetName("DepthPass");
	m_depthPass.SetEnable(true);
    AddPass(&m_depthPass);
	
	// BasePass (Forward).
    m_basePass.SetName("BasePass");
    m_basePass.SetEnable(true);
    AddPass(&m_basePass);

    LOG_INFO(GetInfoAboutPasses().c_str());
}

void Renderer::ClearFrame()
{
    // Clear the back buffer.
    const float m_clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_d3d11->DeviceContext()->ClearRenderTargetView(m_pipelineManager.m_renderTargetView.Get(), m_clearColor);
    m_d3d11->DeviceContext()->ClearDepthStencilView(m_pipelineManager.m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);  
}

void Renderer::Render(Scene* pScene)
{
    if (pScene)
    {
        if (!m_passes.empty())
        {
            for (const auto& pass : m_passes)
            {
                if (pass->IsEnabled())
                {
                    pass->PreRender(m_d3d11->DeviceContext(), &m_pipelineManager);
                    pass->Render(pScene);
                    pass->PostRender(m_d3d11->DeviceContext(), &m_pipelineManager);
                }
            }
        }
    }
}

void Renderer::AddPass(IRenderPass* pass)
{
    m_passes.emplace_back(pass);
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