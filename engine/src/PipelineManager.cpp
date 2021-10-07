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

    // Initialize DepthStencilBuffer.
    if (!this->CreateDepthStencilTexture())
    {
        LOG_ERROR("failed creating DepthStencilBuffer.");
    }

    // Initialize DepthStencilState.
    if (!this->CreateDepthStencilStates())
    {
        LOG_ERROR("failed creating DepthStencilState.");
    }

    // Initialize DepthStencilView.
    if (!this->CreateDepthStencilView())
    {
        LOG_ERROR("failed creating DepthStencilView.");
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
    HRESULT hr = m_d3d11->Device()->CreateRenderTargetView(pBackBuffer, nullptr, m_renderTargetView.GetAddressOf());

    // Release pointer to the back buffer.
    pBackBuffer->Release();

    return !FAILED(hr);
}

bool PipelineManager::CreateDepthStencilTexture()
{
    // Initialize the description of the depth buffer.
    D3D11_TEXTURE2D_DESC depthStencilTextureDesc;
    ZeroMemory(&depthStencilTextureDesc, sizeof(D3D11_TEXTURE2D_DESC));

    // Set up the description of the depth buffer.
    depthStencilTextureDesc.Width = this->m_window->GetWidth();
    depthStencilTextureDesc.Height = this->m_window->GetHeight();
    depthStencilTextureDesc.MipLevels = 1;
    depthStencilTextureDesc.ArraySize = 1;
    depthStencilTextureDesc.SampleDesc.Count = 1;
    depthStencilTextureDesc.SampleDesc.Quality = 0;
    depthStencilTextureDesc.CPUAccessFlags = 0;
    depthStencilTextureDesc.MiscFlags = 0;
    depthStencilTextureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilTextureDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    depthStencilTextureDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;

    // Create the texture for the depth buffer using the filled out description.
    HRESULT hr = m_d3d11->Device()->CreateTexture2D(&depthStencilTextureDesc, nullptr, m_depthStencilTexture.GetAddressOf());

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

bool PipelineManager::CreateDepthStencilView()
{
    // Initialize the depth stencil View.
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

    // Set up the depth stencil View description, setting applied for Deferred Rendering.
    depthStencilViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // Create the depth stencil View.
    HRESULT hr = m_d3d11->Device()->CreateDepthStencilView(m_depthStencilTexture.Get(), &depthStencilViewDesc, m_depthStencilView.GetAddressOf());

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
    m_viewport.Width = static_cast<FLOAT>(this->m_window->GetWidth());
    m_viewport.Height = static_cast<FLOAT>(this->m_window->GetHeight());
    m_viewport.TopLeftX = 0.f;
    m_viewport.TopLeftY = 0.f;

    // Direct3D uses a depth buffer range of 0 to 1, hence:
    m_viewport.MinDepth = 0.f;
    m_viewport.MaxDepth = 1.f;

    // Set viewport.
    m_d3d11->DeviceContext()->RSSetViewports(1, &m_viewport);
}

bool PipelineManager::CreateInputLayouts()
{
    HRESULT hr = S_FALSE;

	// Create m_defaultInputLayout.
	std::string shaderByteCode = m_defaultVertexShader.GetShaderByteCode();
    D3D11_INPUT_ELEMENT_DESC defaultVertexShaderDesc[] =
    {
        {"POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0,                0,                   D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BINORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
	
	if(FAILED(hr = D3D11Core::Get().Device()->CreateInputLayout(defaultVertexShaderDesc, ARRAYSIZE(defaultVertexShaderDesc), shaderByteCode.c_str(), shaderByteCode.length(), &m_defaultInputLayout)))
    {
        LOG_WARNING("failed creating m_defaultInputLayout.");
        return false;
    }

	// Create m_positionOnlyInputLayout.
    shaderByteCode = m_positionOnlyVertexShader.GetShaderByteCode();
    D3D11_INPUT_ELEMENT_DESC positionOnlyVertexShaderDesc[] =
    {
        {"POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0,                0,                   D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    if (FAILED(hr = D3D11Core::Get().Device()->CreateInputLayout(positionOnlyVertexShaderDesc, ARRAYSIZE(positionOnlyVertexShaderDesc), shaderByteCode.c_str(), shaderByteCode.length(), &m_positionOnlyInputLayout)))
    {
        LOG_WARNING("failed creating m_positionOnlyInputLayout.");
        return false;
    }

    return !FAILED(hr);
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
    b.worldMatrix = sm::Matrix::CreateWorld({ 0.f, 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }).Transpose();

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = &b;
    data.SysMemPitch = 0;
    data.SysMemSlicePitch = 0;

    // Model ConstantBuffer
    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&bDesc, &data, m_defaultModelConstantBuffer.GetAddressOf());
    if (FAILED(hr)) return false;

    return !FAILED(hr);
}

bool PipelineManager::CreateTextureEffectConstantBuffer()
{
    D3D11_BUFFER_DESC bDesc = {};
    bDesc.ByteWidth = sizeof(texture_effect_t);
    bDesc.Usage = D3D11_USAGE_DEFAULT;
    bDesc.CPUAccessFlags = 0;
    bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bDesc.MiscFlags = 0;

    texture_effect_t b;
    b.deltaTime = 0;

    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem = &b;
    data.SysMemPitch = 0;
    data.SysMemSlicePitch = 0;

    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&bDesc, &data, m_textureEffectConstantBuffer.GetAddressOf());

    if (FAILED(hr))
    {
        LOG_WARNING("failed to create textureEffectConstantBuffer");
        return false;
    }
    return !FAILED(hr);
}

bool PipelineManager::CreateShaders()
{	
    if (!m_defaultVertexShader.Create("Model_vs"))
    {
        LOG_WARNING("failed creating Model_vs.");
		return false;
    }

    if (!m_positionOnlyVertexShader.Create("depth_vs"))
    {
        LOG_WARNING("failed creating depth_vs.");
        return false;
    }
	
    if(!m_defaultPixelShader.Create("Model_ps"))
    {
        LOG_WARNING("failed creating Model_ps.");
        return false;
    }

    if (!m_textureEffectComputeShader.Create("textureEffect_cs"))
    {
        LOG_WARNING("failed to create textureEffect_cs");
        return false;
    }

    if (!m_textureEffectPixelShader.Create("textureEffect_ps"))
    {
        LOG_WARNING("failed to create textureEffect_ps");
        return false;
    }

    return true;
}                         