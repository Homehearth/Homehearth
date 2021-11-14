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

	AddPass(&m_basePass);		// solo pass.

	AddPass(&m_depthPass);		// 1
	AddPass(&m_frustumPass);	// 2
	AddPass(&m_cullingPass);	// 3
	AddPass(&m_opaqPass);		// 4
	AddPass(&m_transPass);		// 5
	AddPass(&m_animPass);		// 6
	AddPass(&m_decalPass);		// 7
	AddPass(&m_skyPass);		// 8

	m_basePass.SetEnable(true);	// solo pass

	m_depthPass.SetEnable(false);
	m_frustumPass.SetEnable(true);
	m_cullingPass.SetEnable(false);
	m_opaqPass.SetEnable(false);
	m_transPass.SetEnable(false);
	m_decalPass.SetEnable(true);
	m_skyPass.SetEnable(true);

#ifdef _DEBUG
	AddPass(&m_debugPass);
    m_debugPass.SetEnable(true);
#endif

	LOG_INFO("Number of rendering passes: %d", static_cast<int>(m_passes.size()));

	m_decalPass.Create();
	for (const auto& pass : m_passes)
	{
		pass->Initialize(m_d3d11->DeviceContext(), &m_pipelineManager);
	}

	if(!m_frustumPass.UpdateFrustums())
	{
		LOG_WARNING("Failed to initialize FrustumPass.")
	}

}

void Renderer::ClearFrame()
{
    // Clear the back buffer.
    constexpr float m_clearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
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
			if (pScene->GetCurrentCamera()->IsSwapped())
			{
				this->UpdatePerFrame(pScene->GetCurrentCamera());
				thread::RenderThreadHandler::SetCamera(pScene->GetCurrentCamera());
			/*
				Optimize idead: Render/Update lights once instead of per pass?
				Set lights once.
			*/
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
		}
	}
}

void Renderer::OnWindowResize()
{
	// Update frustums.
	const bool enable = m_frustumPass.UpdateFrustums();
	if(!enable)
	{
		LOG_WARNING("Failed to update grid frustums on window resize.")
	}
	m_frustumPass.SetEnable(enable);
}

IRenderPass* Renderer::GetCurrentPass() const
{
	return m_passes[m_currentPass];
}

void Renderer::AddPass(IRenderPass* pass)
{
	m_passes.emplace_back(pass);
}

void Renderer::UpdatePerFrame(Camera* pCam)
{
	// Update Camera constant buffer.
	m_d3d11->DeviceContext()->UpdateSubresource(pCam->m_viewConstantBuffer.Get(), 0,
		nullptr,pCam->GetCameraMatrixes(), 0, 0);

	// todo: if(!m_frustumPass.IsEnabled())
	{
		// Update ScreenToViewParams.
		screen_view_params_t screenToView;
		screenToView.screenDimensions.x = max(m_pipelineManager.m_viewport.Width, 1u);
		screenToView.screenDimensions.y = max(m_pipelineManager.m_viewport.Height, 1u);
		dx::XMStoreFloat4x4(&screenToView.inverseProjection,
			dx::XMMatrixTranspose(dx::XMMatrixInverse(nullptr, pCam->GetProjection())));

		m_pipelineManager.m_screenToViewParamsCB.SetData(m_d3d11->DeviceContext(), screenToView);
	}
}
