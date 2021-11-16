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
    AddPass(&m_frustumPass);    // 2
    AddPass(&m_cullingPass);    // 3
	AddPass(&m_opaqPass);		// 4
	AddPass(&m_transPass);		// 5
	AddPass(&m_animPass);		// 6
	AddPass(&m_decalPass);		// 7
	AddPass(&m_skyPass);		// 8

	m_basePass.SetEnable(false);	// solo pass

	m_depthPass.SetEnable(true);
    m_frustumPass.SetEnable(true);
    m_cullingPass.SetEnable(true);
	m_opaqPass.SetEnable(true);
	m_transPass.SetEnable(true);
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

    m_isForwardPlusInitialized = false;
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

				if (!m_isForwardPlusInitialized)
					InitilializeForwardPlus(pScene->GetCurrentCamera());

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
		nullptr, pCam->GetCameraMatrixes(), 0, 0);
}

void Renderer::InitilializeForwardPlus(Camera* camera)
{
	//
	// Update window dimensions.
	//

	// Safety check: screen resolution must be > 0 so at least 1 thread is created.
    const auto screenWidth = static_cast<uint32_t>(max(m_pipelineManager.m_viewport.Width, 1u));
	const auto screenHeight = static_cast<uint32_t>(max(m_pipelineManager.m_viewport.Height, 1u));

    // We will need 1 frustum for each grid cell.
    // [x, y] screen resolution and [z, z] tile size yield [x/z, y/z] grid size.
    // Resulting in a total of x/z * y/z frustums.
	constexpr uint32_t TILE_SIZE = 16u;
    const uint32_t numFrustums = { screenWidth / TILE_SIZE * screenHeight / TILE_SIZE };

    //
    // Update DispatchParams.
    //
    const dx::XMUINT3 numThreads = { static_cast<uint32_t>(std::ceil(screenWidth / TILE_SIZE)), static_cast<uint32_t>(std::ceil(screenHeight / TILE_SIZE)), 1u };
    const dx::XMUINT3 numThreadGroups = { static_cast<uint32_t>(std::ceil(numThreads.x / TILE_SIZE)), static_cast<uint32_t>(std::ceil(numThreads.y / TILE_SIZE)), 1u };

	m_pipelineManager.m_dispatchParams.numThreadGroups = numThreadGroups;
	m_pipelineManager.m_dispatchParams.numThreads = numThreads;
	m_pipelineManager.m_dispatchParamsCB.SetData(m_d3d11->DeviceContext(), m_pipelineManager.m_dispatchParams);

    const uint32_t COUNT = m_pipelineManager.m_dispatchParams.numThreads.x +
    	m_pipelineManager.m_dispatchParams.numThreads.y + m_pipelineManager.m_dispatchParams.numThreads.z;

    //
    // Update ScreenToViewParams.
    //

    screen_view_params_t screenToView;
    screenToView.screenDimensions.x = static_cast<float>(screenWidth);
    screenToView.screenDimensions.y = static_cast<float>(screenHeight);
    dx::XMStoreFloat4x4(&screenToView.inverseProjection,
        dx::XMMatrixTranspose(dx::XMMatrixInverse(nullptr, camera->GetProjection())));
	m_pipelineManager.m_screenToViewParamsCB.SetData(m_d3d11->DeviceContext(), screenToView);

    //
    // Update GridFrustums.
    //

    m_pipelineManager.m_frustums_data.resize(numFrustums);
    m_pipelineManager.CreateCopyBuffer(m_pipelineManager.m_frustums.buffer.GetAddressOf(), sizeof(frustum_t), m_pipelineManager.m_frustums_data.size());
    m_pipelineManager.CreateStructuredBuffer(m_pipelineManager.m_frustums.buffer.GetAddressOf(), m_pipelineManager.m_frustums_data.data(), sizeof(frustum_t),
		m_pipelineManager.m_frustums_data.size(), m_pipelineManager.m_frustums.uav.GetAddressOf(), m_pipelineManager.m_frustums.srv.GetAddressOf());

    //
    // Create LightGrid.
    //
    if (!CreateLightGridRWB())
    {
        LOG_ERROR("failed to create LightGridRWB")
    }

    //
    // Create LightIndexCounter.
    //
    if (!CreateLightIndexCounterRWB())
    {
        LOG_ERROR("failed to create LightIndexCounterRWB")
    }

    //
    // Create LightIndexList.
    //
    if (!CreateLightIndexListRWB(COUNT))
    {
        LOG_ERROR("failed to create LightIndexListRWB")
    }

    LOG_WARNING("ForwardPlus Initialized")
	m_isForwardPlusInitialized = true;
    m_frustumPass.SetEnable(true);
}


bool Renderer::CreateLightGridRWB()
{
    ID3D11Texture2D* texture2D1 = nullptr;
    ID3D11Texture2D* texture2D2 = nullptr;

    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));

    textureDesc.Width = m_pipelineManager.m_dispatchParams.numThreads.x;
    textureDesc.Height = m_pipelineManager.m_dispatchParams.numThreads.y;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R32G32_UINT;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    textureDesc.MiscFlags = 0;

    HRESULT hr = D3D11Core::Get().Device()->CreateTexture2D(&textureDesc, nullptr, &texture2D1);
    if (FAILED(hr))
        return false;

    hr = D3D11Core::Get().Device()->CreateTexture2D(&textureDesc, nullptr, &texture2D2);
    if (FAILED(hr))
        return false;

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));

    uavDesc.Format = DXGI_FORMAT_R32G32_UINT;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

    srvDesc.Format = DXGI_FORMAT_R32G32_UINT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;

    hr = D3D11Core::Get().Device()->CreateUnorderedAccessView(texture2D1, &uavDesc, m_pipelineManager.opaq_LightGrid.uav.GetAddressOf());
    if (FAILED(hr))
        return false;

    hr = D3D11Core::Get().Device()->CreateUnorderedAccessView(texture2D2, &uavDesc, m_pipelineManager.trans_LightGrid.uav.GetAddressOf());
    if (FAILED(hr))
        return false;

    hr = D3D11Core::Get().Device()->CreateShaderResourceView(texture2D1, &srvDesc, m_pipelineManager.opaq_LightGrid.srv.GetAddressOf());
    if (FAILED(hr))
        return false;

    hr = D3D11Core::Get().Device()->CreateShaderResourceView(texture2D1, &srvDesc, m_pipelineManager.trans_LightGrid.srv.GetAddressOf());
    if (FAILED(hr))
        return false;

    texture2D1->Release();
    texture2D2->Release();

    return !FAILED(hr);
}

bool Renderer::CreateLightIndexListRWB(const uint32_t& COUNT)
{
    const uint32_t SIZE = COUNT * m_pipelineManager.AVERAGE_OVERLAPPING_LIGHTS_PER_TILE;

    m_pipelineManager.opaq_LightIndexList_data.clear();
    m_pipelineManager.trans_LightIndexList_data.clear();

    m_pipelineManager.opaq_LightIndexList_data.resize(SIZE);
    if (!m_pipelineManager.CreateStructuredBuffer(m_pipelineManager.opaq_LightIndexList.buffer.GetAddressOf(), m_pipelineManager.opaq_LightIndexList_data.data(),
        sizeof(UINT), m_pipelineManager.opaq_LightIndexList_data.size(), m_pipelineManager.trans_LightIndexList.uav.GetAddressOf(), m_pipelineManager.opaq_LightIndexList.srv.GetAddressOf()))
    {
        return false;
    }

    m_pipelineManager.trans_LightIndexList_data.resize(SIZE);
    if (!m_pipelineManager.CreateStructuredBuffer(m_pipelineManager.trans_LightIndexList.buffer.GetAddressOf(), m_pipelineManager.trans_LightIndexList_data.data(),
        sizeof(UINT), m_pipelineManager.trans_LightIndexList_data.size(), m_pipelineManager.trans_LightIndexList.uav.GetAddressOf(), m_pipelineManager.trans_LightIndexList.srv.GetAddressOf()))
    {
        return false;
    }

    return true;
}

bool Renderer::CreateLightIndexCounterRWB()
{
    m_pipelineManager.opaq_LightIndexCounter_data.clear();
    m_pipelineManager.opaq_LightIndexCounter_data.clear();

    m_pipelineManager.opaq_LightIndexCounter_data.push_back(0);
    if (!m_pipelineManager.CreateStructuredBuffer(m_pipelineManager.opaq_LightIndexCounter.buffer.GetAddressOf(), m_pipelineManager.opaq_LightIndexCounter_data.data(),
        sizeof(UINT), m_pipelineManager.opaq_LightIndexCounter_data.size(), m_pipelineManager.opaq_LightIndexCounter.uav.GetAddressOf()))
    {
        return false;
    }

    m_pipelineManager.trans_LightIndexCounter_data.push_back(0);
    if (!m_pipelineManager.CreateStructuredBuffer(m_pipelineManager.trans_LightIndexCounter.buffer.GetAddressOf(), m_pipelineManager.trans_LightIndexCounter_data.data(),
        sizeof(UINT), m_pipelineManager.trans_LightIndexCounter_data.size(), m_pipelineManager.opaq_LightIndexCounter.uav.GetAddressOf()))
    {
        return false;
    }

    return true;
}

