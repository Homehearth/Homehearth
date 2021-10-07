#include "EnginePCH.h"
#include "PipelineManager.h"

#include "CommonStructures.h"

PipelineManager::PipelineManager()
	: m_window(nullptr)
	, m_d3d11(nullptr)
	, m_viewport()
{
}

void PipelineManager::Initialize(Window* pWindow)
{
    if (m_window == nullptr)
        m_window = pWindow;

    if (m_d3d11 == nullptr)
        m_d3d11 = &D3D11Core::Get();

    assert((m_window || m_d3d11) || "Renderer could not be initialized.");

    // Initialize RenderTargetView.
    if (!this->CreateRenderTargetView())
    {
        LOG_ERROR("failed creating RenderTargetView.");
    }

    // Initialize DepthStencilState.
    if (!this->CreateDepthStencilStates())
    {
        LOG_ERROR("failed creating DepthStencilState.");
    }

    // Initialize RasterizerStates.
    if (!this->CreateRasterizerStates())
    {
        LOG_ERROR("failed creating RasterizerStates.");
    }

    // Initialize SamplerStates.
    if (!this->CreateSamplerStates())
    {
        LOG_ERROR("failed creating SamplerStates.");
    }

    // Initialize CreateBlendStates.
    if (!this->CreateBlendStates())
    {
        LOG_ERROR("failed creating BlendStates.");
    }

    // Initialize Shaders.
    if (!this->CreateShaders())
    {
        LOG_ERROR("failed creating Shaders.");
    }

    // Initialize InputLayouts.
    if (!this->CreateInputLayouts())
    {
        LOG_ERROR("failed creating InputLayouts.");
    }

    // Initialize ConstantBuffers (temp?).
    if (!this->CreateDefaultConstantBuffer())
    {
        LOG_ERROR("failed creating default constant buffer.");
    }

    // Initialize CreateDepthMap.
    if (!this->CreateDepthMap())
    {
        LOG_ERROR("failed creating depth map.");
    }

	if(!CreateStructuredBuffers())
    {
        LOG_ERROR("failed creating structured buffers.");
    }
	
    // Set Viewport.
    this->SetViewport();
}

bool PipelineManager::CreateRenderTargetView()
{
    ID3D11Texture2D* pBackBuffer = nullptr;

    // Get the pointer to the back buffer.
    if (FAILED(m_d3d11->SwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pBackBuffer))))
        return false;

    // Create the renderTargetView with the back buffer pointer.
    HRESULT hr = m_d3d11->Device()->CreateRenderTargetView(pBackBuffer, nullptr, m_backBufferTarget.GetAddressOf());

    // Release pointer to the back buffer.
    pBackBuffer->Release();

    return !FAILED(hr);
}

bool PipelineManager::CreateDepthStencilStates()
{
    // Initialize the description of the stencil state.
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

    // Set up the description of the stencil state.
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.StencilReadMask = 0xFF;
    depthStencilDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing.
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing.
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Create m_depthStencilStateLess.
    HRESULT hr = m_d3d11->Device()->CreateDepthStencilState(&depthStencilDesc, m_depthStencilStateLess.GetAddressOf());
    if (FAILED(hr))
        return false;
	
    // Create m_depthStencilStateGreater.
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;
    hr = m_d3d11->Device()->CreateDepthStencilState(&depthStencilDesc, m_depthStencilStateGreater.GetAddressOf());
    if (FAILED(hr))
        return false;

	// Create m_depthStencilStateEqualAndDisableDepthWrite.
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
    hr = m_d3d11->Device()->CreateDepthStencilState(&depthStencilDesc, m_depthStencilStateEqualAndDisableDepthWrite.GetAddressOf());
	
    return !FAILED(hr);
}

bool PipelineManager::CreateRasterizerStates()
{
    // Initialize the description of the rasterizer state.
    D3D11_RASTERIZER_DESC rasterizerDesc;
    ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

    // Setup the raster description: how and what polygons will be drawn.
    rasterizerDesc.DepthClipEnable = true;
    rasterizerDesc.AntialiasedLineEnable = false;
    rasterizerDesc.FrontCounterClockwise = false;
    rasterizerDesc.MultisampleEnable = false;
    rasterizerDesc.ScissorEnable = false;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.DepthBiasClamp = 0.0f;
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;
    rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    // Backface culling refers to the process of discarding back-facing triangles from the pipeline. 
    // This can potentially reduce the amount of triangles that need to be processed by half, hence it will be set as default.
    rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;

    // Create the rasterizer state from the description we just filled out.
    HRESULT hr = m_d3d11->Device()->CreateRasterizerState(&rasterizerDesc, m_rasterState.GetAddressOf());

    // Setup a raster description with no back face culling.
    rasterizerDesc.CullMode = D3D11_CULL_NONE;

    // Create the no culling rasterizer state.
    hr = m_d3d11->Device()->CreateRasterizerState(&rasterizerDesc, m_rasterStateNoCulling.GetAddressOf());
    if (FAILED(hr))
        return false;

    // Setup a raster description which enables wire frame rendering.
    rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
    rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;

    // Create the wire frame rasterizer state.
    hr = m_d3d11->Device()->CreateRasterizerState(&rasterizerDesc, m_rasterStateWireframe.GetAddressOf());

    return !FAILED(hr);
}

bool PipelineManager::CreateSamplerStates()
{
    // Create a sampler state description.
    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));

    // Setup for Linear SamplerState
    samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    samplerDesc.MinLOD = 0;

    HRESULT hr = D3D11Core::Get().Device()->CreateSamplerState(&samplerDesc, m_linearSamplerState.GetAddressOf());
    if (FAILED(hr))
        return false;

    // Setup for Point SamplerState
    samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.MinLOD = 0;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;

    hr = D3D11Core::Get().Device()->CreateSamplerState(&samplerDesc, m_pointSamplerState.GetAddressOf());

    return !FAILED(hr);
}

bool PipelineManager::CreateBlendStates()
{
    // Create a blend state description. 
    D3D11_BLEND_DESC blendStateDesc;
    ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));

    D3D11_RENDER_TARGET_BLEND_DESC rtbd;
    ZeroMemory(&rtbd, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));
	
    // Setup for Opaque BlendState.
    rtbd.BlendEnable = 0;
    rtbd.BlendOp = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlend = D3D11_BLEND_ONE;
    rtbd.DestBlend = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendStateDesc.RenderTarget[0] = rtbd;
    blendStateDesc.AlphaToCoverageEnable = 0;
    blendStateDesc.IndependentBlendEnable = 0;

    // Create m_blendStatepOpaque.
    HRESULT hr = m_d3d11->Device()->CreateBlendState(&blendStateDesc, m_blendStatepOpaque.GetAddressOf());
    if (FAILED(hr))
        return false;

    // Create m_blendStatepDepthOnlyAlphaTest.
    blendStateDesc.RenderTarget[0].RenderTargetWriteMask = 0;
    hr = m_d3d11->Device()->CreateBlendState(&blendStateDesc, m_blendStatepDepthOnlyAlphaTest.GetAddressOf());
    if (FAILED(hr))
        return false;

    // Create m_blendStateDepthOnlyAlphaToCoverage.
	// "...the quality is significantly improved when used in conjunction with MSAA".
    blendStateDesc.AlphaToCoverageEnable = 1;
    hr = m_d3d11->Device()->CreateBlendState(&blendStateDesc, m_blendStateDepthOnlyAlphaToCoverage.GetAddressOf());

	return !FAILED(hr);
}

void PipelineManager::SetViewport()
{
    // Initialize the viewport to occupy the entire client area.
    RECT clientRect;
    GetClientRect(this->m_window->GetHWnd(), &clientRect);
    const LONG clientWidth = clientRect.right - clientRect.left;
    const LONG clientHeight = clientRect.bottom - clientRect.top;

    ZeroMemory(&m_viewport, sizeof(D3D11_VIEWPORT));

    // Setup the viewport for rendering.
    m_viewport.Width = static_cast<FLOAT>(m_window->GetWidth());
    m_viewport.Height = static_cast<FLOAT>(m_window->GetHeight());
    m_viewport.TopLeftX = 0.f;
    m_viewport.TopLeftY = 0.f;

    // Direct3D uses a depth buffer range of 0 to 1, hence:
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;

    // Set viewport.
    m_d3d11->DeviceContext()->RSSetViewports(1, &m_viewport);
}

bool PipelineManager::CreateInputLayouts()
{
    HRESULT hr = S_FALSE;

	// Create m_defaultInputLayout.
	std::string shaderByteCode = m_defaultVertexShader.GetShaderByteCode();
    D3D11_INPUT_ELEMENT_DESC defaultInputLayoutDesc[] =
    {
        {"POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0,                0,                   D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BINORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
	if(FAILED(hr = m_d3d11->Device()->CreateInputLayout(defaultInputLayoutDesc, ARRAYSIZE(defaultInputLayoutDesc), shaderByteCode.c_str(), shaderByteCode.length(), m_defaultInputLayout.GetAddressOf())))
    {
        LOG_WARNING("failed creating m_defaultInputLayout.");
        return false;
    }
	
    return !FAILED(hr);
}

bool PipelineManager::CreateDepthMap()
{
    // Initialize the description of the depth buffer.
    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));

    // Set up the description of the depth buffer.
    textureDesc.Width = m_window->GetWidth();
    textureDesc.Height = m_window->GetHeight();
    textureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS;
    textureDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    // Create the texture for the depth buffer using the filled out description.
    ID3D11Texture2D* pDepthStencilTexture;
    HRESULT hr = m_d3d11->Device()->CreateTexture2D(&textureDesc, nullptr, &pDepthStencilTexture);
    if (FAILED(hr))
        return false;

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));

    depthStencilViewDesc.Flags = 0;
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    hr = m_d3d11->Device()->CreateDepthStencilView(pDepthStencilTexture, &depthStencilViewDesc, m_depthStencilView.GetAddressOf());
    if (FAILED(hr))
        return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;

    hr = m_d3d11->Device()->CreateShaderResourceView(pDepthStencilTexture, &shaderResourceViewDesc, m_depthStencilSRV.GetAddressOf());

    if (pDepthStencilTexture != nullptr)
        pDepthStencilTexture->Release();
	
    return !FAILED(hr);
}

bool PipelineManager::CreateStructuredBuffers()
{


    return true;
}

bool PipelineManager::CreateDefaultConstantBuffer()
{
    D3D11_BUFFER_DESC bDesc;
    bDesc.ByteWidth = sizeof(basic_model_matrix_t);
    bDesc.Usage = D3D11_USAGE_DEFAULT;
    bDesc.CPUAccessFlags = 0;
    bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bDesc.MiscFlags = 0;

    basic_model_matrix_t b;
    b.worldMatrix = sm::Matrix::CreateWorld({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }).Transpose();

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = &b;
    data.SysMemPitch = 0;
    data.SysMemSlicePitch = 0;

    // Model ConstantBuffer
    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&bDesc, &data, m_defaultModelConstantBuffer.GetAddressOf());
    if (FAILED(hr)) return false;

    return !FAILED(hr);
}

bool PipelineManager::CreateShaders()
{	
    if (!m_defaultVertexShader.Create("Model_vs"))
    {
        LOG_WARNING("failed creating Model_vs.");
		return false;
    }

    if (!m_depthVertexShader.Create("Depth_vs"))
    {
        LOG_WARNING("failed creating Depth_vs.");
        return false;
    }
	
    if(!m_defaultPixelShader.Create("Model_ps"))
    {
        LOG_WARNING("failed creating Model_ps.");
        return false;
    }

    if (!m_depthPixelShader.Create("Depth_ps"))
    {
        LOG_WARNING("failed creating Depth_ps.");
        return false;
    }

    return true;
}                         