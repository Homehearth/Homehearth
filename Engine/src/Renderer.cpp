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

	/*
		Had to disable the depth pass to get alpha testing to work correctly... -Filip
	*/
	//AddPass(&m_depthPass);  // 1
	AddPass(&m_basePass);   // 2
	AddPass(&m_animPass);	// 3

	//m_depthPass.SetEnable(true);
	m_basePass.SetEnable(true);
	m_animPass.SetEnable(true);

#ifdef _DEBUG
	AddPass(&m_debugPass);  // 4
    m_debugPass.SetEnable(true);
#endif

	LOG_INFO("Number of rendering passes: %d", static_cast<int>(m_passes.size()));

	for (auto& pass : m_passes)
	{
		pass->Initialize(m_d3d11->DeviceContext(), &m_pipelineManager);
	}
}

void Renderer::ClearFrame()
{
    // Clear the back buffer.
    const float m_clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
    m_d3d11->DeviceContext()->ClearRenderTargetView(m_pipelineManager.m_backBuffer.Get(), m_clearColor);
    m_d3d11->DeviceContext()->ClearDepthStencilView(m_pipelineManager.m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    m_d3d11->DeviceContext()->ClearDepthStencilView(m_pipelineManager.m_debugDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderer::Render(Scene* pScene)
{
	if (pScene)
	{
		if (!m_passes.empty())
		{
			//Update and bind the camera once
			pScene->GetCurrentCamera()->Update();
			pScene->GetCurrentCamera()->BindCB();
			thread::RenderThreadHandler::SetCamera(pScene->GetCurrentCamera());
			/*
				Optimize idea: Render/Update lights once instead of per pass?
				Set lights once.
			*/
			for (int i = 0; i < m_passes.size(); i++)
			{
				m_currentPass = i;
				IRenderPass* pass = m_passes[i];
				if (pass->IsEnabled())
				{
					pass->SetLights(pScene->GetLights());
					pass->PreRender();
					pass->Render(pScene);
					pass->PostRender();
				}
			}

			pScene->ReadyForSwap();
			pScene->GetCurrentCamera()->UnbindCB();
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
