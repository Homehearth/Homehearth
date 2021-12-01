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

	//AddPass(&m_depthPass);  
	AddPass(&m_shadowPass);
	m_shadowPass.StartUp();

	AddPass(&m_decalPass);
	m_decalPass.Create();

	AddPass(&m_basePass);   
	AddPass(&m_animPass);	
	AddPass(&m_skyPass);
	AddPass(&m_dofPass);	
	AddPass(&m_particlePass);

	m_basePass.m_pShadowPass = &m_shadowPass;
	m_animPass.m_pShadowPass = &m_shadowPass;

	//m_depthPass.SetEnable(true);
	m_basePass.SetEnable(true);
	m_animPass.SetEnable(true);
	m_decalPass.SetEnable(true);
	m_particlePass.SetEnable(true);
	m_skyPass.SetEnable(true);
	m_dofPass.SetEnable(true);
	m_shadowPass.SetEnable(true);

#ifdef _DEBUG
	AddPass(&m_debugPass);  
    m_debugPass.SetEnable(true);
#endif

	LOG_INFO("Number of rendering passes: %d", static_cast<int>(m_passes.size()));

	for (auto& pass : m_passes)
	{
		pass->Initialize(m_d3d11->DeviceContext(), &m_pipelineManager);
	}

	m_dofPass.Create(DoFType::VIGNETTE);
}

void Renderer::Setup(BasicEngine<Scene>& engine)
{
	/*
	engine.GetScene("Game").ForEachComponent<comp::Light>([&](comp::Light& l) {

		m_shadowPass.CreateShadow(l);

		});

	m_shadowPass.SetupMap();
	*/

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
			m_basePass.m_skyboxRef = pScene->GetSkybox();
			m_animPass.m_skyboxRef = pScene->GetSkybox();
			if (pScene->GetCurrentCamera()->IsSwapped())
			{
				this->UpdatePerFrame(pScene->GetCurrentCamera());
				thread::RenderThreadHandler::SetCamera(pScene->GetCurrentCamera());

				for (int i = 0; i < m_passes.size(); i++)
				{
					m_currentPass = i;
					IRenderPass* pass = m_passes[i];
					if (pass->IsEnabled())
					{
						pass->SetLights(pScene->GetLights());
						pass->PreRender(pScene->GetCurrentCamera());
						pass->Render(pScene);
						pass->PostRender();
					}
				}

				pScene->GetCurrentCamera()->ReadySwap();
				pScene->ReadyForSwap();
			}
			else
			{
				pScene->GetCurrentCamera()->ReadySwap();
				pScene->ReadyForSwap();
			}
		}
	}
}

IRenderPass* Renderer::GetCurrentPass() const
{
	return m_passes[m_currentPass];
}

DOFPass* Renderer::GetDoFPass()
{
	return &m_dofPass;
}

ShadowPass* Renderer::GetShadowPass()
{
	return &m_shadowPass;
}

void Renderer::SetShadowMapSize(uint32_t size)
{
	m_shadowPass.SetShadowMapSize(size);
}

uint32_t Renderer::GetShadowMapSize() const
{
	return m_shadowPass.GetShadowMapSize();
}

void Renderer::ImGuiShowTextures()
{
	m_shadowPass.ImGuiShowTextures();
}

void Renderer::AddPass(IRenderPass* pass)
{
	m_passes.emplace_back(pass);
}

void Renderer::UpdatePerFrame(Camera* pCam)
{
	if (pCam)
	{
		// Update Camera constant buffer.
		m_d3d11->DeviceContext()->UpdateSubresource(pCam->m_viewConstantBuffer.Get(), 0, nullptr, pCam->GetCameraMatrixes(), 0, 0);
	}
}