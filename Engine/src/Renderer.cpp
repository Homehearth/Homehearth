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
    AddPass(&m_frustumPass);
    AddPass(&m_cullingPass);
    AddPass(&m_textureEffectPass);
	AddPass(&m_waterEffectPass);
    
	//AddPass(&m_depthPass);  
	AddPass(&m_shadowPass);
	m_shadowPass.StartUp();

	AddPass(&m_decalPass); // måste vara före basepass
	m_decalPass.Create();

	AddPass(&m_basePass);   
	AddPass(&m_animPass);	
	AddPass(&m_skyPass);
	AddPass(&m_dofPass);	
	AddPass(&m_particlePass);

	m_basePass.m_pShadowPass = &m_shadowPass;
	m_animPass.m_pShadowPass = &m_shadowPass;

	m_depthPass.SetEnable(true);
    m_cullingPass.SetEnable(true);
	m_basePass.SetEnable(true);
	m_animPass.SetEnable(true);
	m_decalPass.SetEnable(true);
	m_particlePass.SetEnable(true);
	m_skyPass.SetEnable(true);
	m_dofPass.SetEnable(true);
	m_shadowPass.SetEnable(true);
	m_textureEffectPass.SetEnable(true);
	m_waterEffectPass.SetEnable(true);

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
    m_isForwardPlusInitialized = false;
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
			m_particlePass.m_skyboxRef = pScene->GetSkybox();
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
			}

			pScene->GetCurrentCamera()->ReadySwap();
			pScene->ReadyForSwap();
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
		m_d3d11->DeviceContext()->UpdateSubresource(pCam->m_viewConstantBuffer.Get(),
			0, nullptr, pCam->GetCameraMatrixes(), 0, 0);

        if (!m_isForwardPlusInitialized)
            InitilializeForwardPlus(pCam);
	}
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
    const uint32_t numFrustums = { (screenWidth / TILE_SIZE) * (screenHeight / TILE_SIZE) };

    //
    // Update DispatchParams.
    //

    const dx::XMUINT4 numThreads = { static_cast<uint32_t>(std::ceil(screenWidth / TILE_SIZE)), static_cast<uint32_t>(std::ceil(screenHeight / TILE_SIZE)), 1u, 1u };
    const dx::XMUINT4 numThreadGroups = { static_cast<uint32_t>(std::ceil(numThreads.x / TILE_SIZE)), static_cast<uint32_t>(std::ceil(numThreads.y / TILE_SIZE)), 1u, 1u };

    m_pipelineManager.m_dispatchParams.numThreadGroups = numThreadGroups;
    m_pipelineManager.m_dispatchParams.numThreads = numThreads;
    m_pipelineManager.m_dispatchParamsCB.SetData(m_d3d11->DeviceContext(), m_pipelineManager.m_dispatchParams);

    //
    // Update ScreenToViewParams.
    //

    m_pipelineManager.m_screenToViewParams.screenDimensions.x = static_cast<float>(screenWidth);
    m_pipelineManager.m_screenToViewParams.screenDimensions.y = static_cast<float>(screenHeight);
    dx::XMStoreFloat4x4(&m_pipelineManager.m_screenToViewParams.inverseProjection,
        dx::XMMatrixTranspose(dx::XMMatrixInverse(nullptr, camera->GetProjection())));
    m_pipelineManager.m_screenToViewParamsCB.SetData(m_d3d11->DeviceContext(), m_pipelineManager.m_screenToViewParams);

    //
    // Update GridFrustums.
    //

    const uint32_t numGridCells = m_pipelineManager.m_dispatchParams.numThreads.x *
        m_pipelineManager.m_dispatchParams.numThreads.y * m_pipelineManager.m_dispatchParams.numThreads.z;
    m_pipelineManager.m_frustums_data.resize(numFrustums);
    m_pipelineManager.CreateStructuredBuffer(m_pipelineManager.m_frustums_data.data(),
        sizeof(frustum_t), m_pipelineManager.m_frustums_data.size(), m_pipelineManager.m_frustums);

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
    if (!CreateLightIndexListRWB(numGridCells))
    {
        LOG_ERROR("failed to create LightIndexListRWB")
    }

    //
    // Create DebugTexture.
    //
    if (!CreateHeatMapRWB())
    {
        LOG_ERROR("failed to create HeatMapRWB")
    }

    LOG_WARNING("ForwardPlus Initialized")
	m_isForwardPlusInitialized = true;
    m_frustumPass.SetEnable(true);
}


bool Renderer::CreateLightGridRWB()
{
    ComPtr<ID3D11Texture2D> texture2D1 = nullptr;
    ComPtr<ID3D11Texture2D> texture2D2 = nullptr;

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

    HRESULT hr = D3D11Core::Get().Device()->CreateTexture2D(&textureDesc, nullptr, texture2D1.GetAddressOf());
    if (FAILED(hr))
        return false;

    hr = D3D11Core::Get().Device()->CreateTexture2D(&textureDesc, nullptr, texture2D2.GetAddressOf());
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

    hr = D3D11Core::Get().Device()->CreateUnorderedAccessView(texture2D1.Get(), &uavDesc, m_pipelineManager.opaq_LightGrid.uav.GetAddressOf());
    if (FAILED(hr))
        return false;

    hr = D3D11Core::Get().Device()->CreateUnorderedAccessView(texture2D2.Get(), &uavDesc, m_pipelineManager.trans_LightGrid.uav.GetAddressOf());
    if (FAILED(hr))
        return false;

    hr = D3D11Core::Get().Device()->CreateShaderResourceView(texture2D1.Get(), &srvDesc, m_pipelineManager.opaq_LightGrid.srv.GetAddressOf());
    if (FAILED(hr))
        return false;

    hr = D3D11Core::Get().Device()->CreateShaderResourceView(texture2D2.Get(), &srvDesc, m_pipelineManager.trans_LightGrid.srv.GetAddressOf());
    if (FAILED(hr))
        return false;

    return !FAILED(hr);
}

bool Renderer::CreateLightIndexListRWB(const uint32_t& COUNT)
{
    // Size of the light index list: for a screen resolution of 1280x720 and
    // a tile size of 16�16 results in a 80x45 (3, 600) light grid.
    // Assuming an average of 100 lights per tile,
    // this would require a light index list of 360,000 indices.
    // Each light index cost 4 bytes (for a 32 - bit unsigned integer)
    // so the light list would consume 1.44 MB of GPU memory. Since we
    // need two lists, one for opaque and one for transparent, the memory
    // usage will double.

    // Size of the light index list:
    const uint32_t SIZE = COUNT * m_pipelineManager.AVERAGE_OVERLAPPING_LIGHTS_PER_TILE;

    m_pipelineManager.opaq_LightIndexList_data.clear();
    m_pipelineManager.trans_LightIndexList_data.clear();

    m_pipelineManager.opaq_LightIndexList_data.resize(SIZE);
    if (!m_pipelineManager.CreateStructuredBuffer(m_pipelineManager.opaq_LightIndexList_data.data(), sizeof(UINT), m_pipelineManager.opaq_LightIndexList_data.size(), m_pipelineManager.trans_LightIndexList))
    {
        return false;
    }

    m_pipelineManager.trans_LightIndexList_data.resize(SIZE);
    if (!m_pipelineManager.CreateStructuredBuffer(m_pipelineManager.trans_LightIndexList_data.data(), sizeof(UINT), m_pipelineManager.trans_LightIndexList_data.size(), m_pipelineManager.trans_LightIndexList))
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
    if (!m_pipelineManager.CreateStructuredBuffer(m_pipelineManager.opaq_LightIndexCounter.buffer, m_pipelineManager.opaq_LightIndexCounter_data.data(),
        sizeof(UINT), m_pipelineManager.opaq_LightIndexCounter_data.size(), m_pipelineManager.opaq_LightIndexCounter.uav))
    {
        return false;
    }

    m_pipelineManager.trans_LightIndexCounter_data.push_back(0);
    if (!m_pipelineManager.CreateStructuredBuffer(m_pipelineManager.trans_LightIndexCounter.buffer, m_pipelineManager.trans_LightIndexCounter_data.data(),
        sizeof(UINT), m_pipelineManager.trans_LightIndexCounter_data.size(), m_pipelineManager.opaq_LightIndexCounter.uav))
    {
        return false;
    }

    return true;
}

bool Renderer::CreateHeatMapRWB()
{
    ComPtr<ID3D11Texture2D> texture2D = nullptr;

    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));

    textureDesc.Width = m_pipelineManager.m_dispatchParams.numThreads.x;
    textureDesc.Height = m_pipelineManager.m_dispatchParams.numThreads.y;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    textureDesc.MiscFlags = 0;

    HRESULT hr = D3D11Core::Get().Device()->CreateTexture2D(&textureDesc, nullptr, texture2D.GetAddressOf());
    if (FAILED(hr))
        return false;

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));

    uavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

    srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;

    hr = D3D11Core::Get().Device()->CreateUnorderedAccessView(texture2D.Get(), &uavDesc, m_pipelineManager.m_heatMap.uav.GetAddressOf());
    if (FAILED(hr))
        return false;

    hr = D3D11Core::Get().Device()->CreateShaderResourceView(texture2D.Get(), &srvDesc, m_pipelineManager.m_heatMap.srv.GetAddressOf());
    if (FAILED(hr))
        return false;

    return !FAILED(hr);
}
