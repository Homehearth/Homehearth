#include "EnginePCH.h"
#include "PipelineManager.h"
#include "CommonStructures.h"

PipelineManager::PipelineManager()
    : m_window(nullptr)
    , m_d3d11(&D3D11Core::Get())
    , m_viewport()
{
    m_windowWidth = 0;
    m_windowHeight = 0;

    m_context = nullptr;
}

void PipelineManager::Initialize(Window* pWindow, ID3D11DeviceContext* context)
{
    if (context)
        m_context = context;
    else
        m_context = nullptr;

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

    // Initialize DepthBuffer.
    if (!this->CreateDepthBuffer())
    {
        LOG_ERROR("failed creating DepthBuffer.");
    }

    // Initialize DepthStencilState.
    if (!this->CreateDepthStencilStates())
    {
        LOG_ERROR("failed creating DepthStencilStates.");
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
    if (!this->CreateTextureEffectConstantBuffer())
    {
        LOG_ERROR("failed creating texture effect constant buffer.");
    }

    if (!this->CreateTextureEffectResources())
    {
        LOG_ERROR("failed creating texture effect resources.");
    }

    // Initialize Forward+ related resources.
    {
        if (!this->CreateDepth())
        {
            LOG_ERROR("failed creating depth for pre-pass.");
        }
		if (!this->CreateForwardBlendStates())
		{
			LOG_ERROR("failed creating blend states.");
		}   
        m_dispatchParamsCB.Create(m_d3d11->Device());
        m_screenToViewParamsCB.Create(m_d3d11->Device());
    }

    // Set Viewport.
    this->SetViewport();
    m_windowWidth = m_window->GetWidth();
    m_windowHeight = m_window->GetHeight();
}


bool PipelineManager::CreateStructuredBuffer(ComPtr<ID3D11Buffer>& buffer, void* data, unsigned int byteStride,
    unsigned int arraySize, ComPtr<ID3D11UnorderedAccessView>& uav)
{
    buffer.Reset();
    uav.Reset();

    D3D11_BUFFER_DESC sBufferDesc = {};
    D3D11_SUBRESOURCE_DATA sBufferSub = {};

    sBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    sBufferDesc.ByteWidth = byteStride * arraySize;
    sBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    sBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    sBufferDesc.StructureByteStride = byteStride;
    sBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    sBufferSub.pSysMem = data;

    HRESULT hr = m_d3d11->Device()->CreateBuffer(&sBufferDesc, &sBufferSub, buffer.GetAddressOf());
    if (FAILED(hr))
        return false;

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.Flags = 0;
    uavDesc.Buffer.NumElements = arraySize;
    hr = m_d3d11->Device()->CreateUnorderedAccessView(buffer.Get(), &uavDesc, uav.GetAddressOf());

    return !FAILED(hr);
}

bool PipelineManager::CreateStructuredBuffer(void* data, unsigned byteStride, unsigned arraySize, ResourceAccessView& rav)
{
    rav.buffer.Reset();
    rav.uav.Reset();
    rav.srv.Reset();

    D3D11_BUFFER_DESC sBufferDesc = {};
    D3D11_SUBRESOURCE_DATA sBufferSub = {};

    sBufferDesc.ByteWidth = byteStride * arraySize;
    sBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    sBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    sBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    sBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    sBufferDesc.StructureByteStride = byteStride;
    sBufferSub.pSysMem = data;

    HRESULT hr = m_d3d11->Device()->CreateBuffer(&sBufferDesc, &sBufferSub, rav.buffer.GetAddressOf());
    if (FAILED(hr))
        return false;

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.Flags = 0;
    uavDesc.Buffer.NumElements = arraySize;

    hr = m_d3d11->Device()->CreateUnorderedAccessView(rav.buffer.Get(), &uavDesc, rav.uav.GetAddressOf());
    if (FAILED(hr))
        return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    srvDesc.BufferEx.FirstElement = 0;
    srvDesc.BufferEx.Flags = 0;
    srvDesc.BufferEx.NumElements = arraySize;
    hr = m_d3d11->Device()->CreateShaderResourceView(rav.buffer.Get(), &srvDesc, rav.srv.GetAddressOf());

    return !FAILED(hr);
}

void PipelineManager::SetCullBack(bool cullNone, ID3D11DeviceContext* pDeviceContext)
{
	constexpr float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    if (!cullNone)
    {
        pDeviceContext->RSSetState(m_rasterStateNoCulling.Get());
        pDeviceContext->OMSetBlendState(m_blendOn.Get(), blendFactor, 0xffffffff);
    }
    else
    {
        pDeviceContext->RSSetState(m_rasterStateBackCulling.Get());
        pDeviceContext->OMSetBlendState(m_blendOff.Get(), blendFactor, 0xffffffff);
    }
}

bool PipelineManager::CreateRenderTargetView()
{
    ID3D11Texture2D* pBackBuffer = nullptr;

    // Get the pointer to the back buffer.
    if (FAILED(m_d3d11->SwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pBackBuffer))))
        return false;

    // Create the renderTargetView with the back buffer pointer.
    HRESULT hr = m_d3d11->Device()->CreateRenderTargetView(pBackBuffer, nullptr, m_backBuffer.GetAddressOf());

    // Release pointer to the back buffer.
    pBackBuffer->Release();

    return !FAILED(hr);
}

bool PipelineManager::CreateDepthBuffer()
{
    D3D11_TEXTURE2D_DESC depthBufferDesc;
    ZeroMemory(&depthBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));
    depthBufferDesc.Width = m_window->GetWidth();
    depthBufferDesc.Height = m_window->GetHeight();
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    depthBufferDesc.CPUAccessFlags = 0;
    depthBufferDesc.MiscFlags = 0;
    HRESULT hr = m_d3d11->Device()->CreateTexture2D(&depthBufferDesc, nullptr, m_depthStencilTexture.GetAddressOf());
    if (FAILED(hr))
        return false;

    hr = m_d3d11->Device()->CreateTexture2D(&depthBufferDesc, nullptr, m_debugDepthStencilTexture.GetAddressOf());
    if (FAILED(hr))
        return false;

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;
    hr = m_d3d11->Device()->CreateDepthStencilView(m_depthStencilTexture.Get(), &depthStencilViewDesc, m_depthStencilView.GetAddressOf());
    if (FAILED(hr))
        return false;

    //Depth for debug render
    hr = m_d3d11->Device()->CreateDepthStencilView(m_debugDepthStencilTexture.Get(), &depthStencilViewDesc, m_debugDepthStencilView.GetAddressOf());
    if (FAILED(hr))
        return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;

    hr = m_d3d11->Device()->CreateShaderResourceView(m_depthStencilTexture.Get(), &shaderResourceViewDesc, m_depthBufferSRV.GetAddressOf());
    if (FAILED(hr))
        return false;

    hr = m_d3d11->Device()->CreateShaderResourceView(m_debugDepthStencilTexture.Get(), &shaderResourceViewDesc, m_debugDepthBufferSRV.GetAddressOf());

    return !FAILED(hr);
}

bool PipelineManager::CreateDepthStencilStates()
{
    // Initialize the description of the stencil state.
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

    // Set up the description of the stencil state (Less).
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.StencilEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    depthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

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

    // Create m_depthStencilStateLessOrEqual.
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    hr = m_d3d11->Device()->CreateDepthStencilState(&depthStencilDesc, m_depthStencilStateLessOrEqual.GetAddressOf());
    if (FAILED(hr))
        return false;


    // Create m_depthStencilStateLessOrEqualEnableDepthWrite.
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    hr = m_d3d11->Device()->CreateDepthStencilState(&depthStencilDesc, m_depthStencilStateLessOrEqualEnableDepthWrite.GetAddressOf());
    if (FAILED(hr))
        return false;

    // Create m_depthStencilStateGreater
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthEnable = false;
    hr = m_d3d11->Device()->CreateDepthStencilState(&depthStencilDesc, m_depthStencilStateGreater.GetAddressOf());
    if (FAILED(hr))
        return false;


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
    rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
    // Create the rasterizer state from the description we just filled out.
    HRESULT hr = m_d3d11->Device()->CreateRasterizerState(&rasterizerDesc, m_rasterStateBackCulling.GetAddressOf());


    rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_FRONT;
    // Create front face culling rasterizer state.
    hr = m_d3d11->Device()->CreateRasterizerState(&rasterizerDesc, m_rasterStateFrontCulling.GetAddressOf());

    // Setup a raster description with no back face culling.
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
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

    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_CLAMP;

    hr = D3D11Core::Get().Device()->CreateSamplerState(&samplerDesc, m_linearClampSamplerState.GetAddressOf());
    if (FAILED(hr))
        return false;

    // Setup for Anisotropic SamplerState
    samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    samplerDesc.MinLOD = 0.f;
    samplerDesc.MipLODBias = 0.f;
    samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;

    hr = D3D11Core::Get().Device()->CreateSamplerState(&samplerDesc, m_anisotropicSamplerState.GetAddressOf());
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
    if (FAILED(hr))
        return false;

    // Setup for Cubemap SamplerState
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 4;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = D3D11Core::Get().Device()->CreateSamplerState(&samplerDesc, m_cubemapSamplerState.GetAddressOf());

    return !FAILED(hr);
}

bool PipelineManager::CreateBlendStates()
{
    // Create a blend state description. 
    D3D11_BLEND_DESC blendStateDesc;
    ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));

    D3D11_RENDER_TARGET_BLEND_DESC rtbd;
    ZeroMemory(&rtbd, sizeof(D3D11_RENDER_TARGET_BLEND_DESC));

    // Setup Opaque blend state
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

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;

    HRESULT hr = m_d3d11->Device()->CreateBlendState(&blendDesc, m_blendStateAlphaBlending.GetAddressOf());
    if (FAILED(hr))
        return false;

    // Create m_blendStatepOpaque.
    hr = m_d3d11->Device()->CreateBlendState(&blendStateDesc, m_blendStatepOpaque.GetAddressOf());
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
    if (FAILED(hr))
        return false;

    ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
    blendDesc.AlphaToCoverageEnable = true;
    blendDesc.IndependentBlendEnable = true;
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = m_d3d11->Device()->CreateBlendState(&blendDesc, m_alphaBlending.GetAddressOf());
    if (FAILED(hr))
        return false;

    D3D11_BLEND_DESC blendStateParticleDesc = {};
    blendStateParticleDesc.AlphaToCoverageEnable = true;
    blendStateParticleDesc.IndependentBlendEnable = true;
    blendStateParticleDesc.RenderTarget[0].BlendEnable = true;
    blendStateParticleDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; //Blend opacity: add 1 and 2
    blendStateParticleDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD; //Blend alpha: add 1 and 2
    blendStateParticleDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendStateParticleDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO; //No blend
    blendStateParticleDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendStateParticleDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendStateParticleDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;

    hr = m_d3d11->Device()->CreateBlendState(&blendStateParticleDesc, m_blendStateParticle.GetAddressOf());


    if (FAILED(hr))
    {
        LOG_ERROR("Couldnt create particle UAV");
        return false;
    }

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
    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;

    // Direct3D uses a depth buffer range of 0 to 1, hence:
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;

    // Set viewport.
    if (m_context)
        m_context->RSSetViewports(1, &m_viewport);
    else
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

    if (FAILED(hr = D3D11Core::Get().Device()->CreateInputLayout(defaultVertexShaderDesc, ARRAYSIZE(defaultVertexShaderDesc), shaderByteCode.c_str(), shaderByteCode.length(), &m_defaultInputLayout)))
    {
        LOG_WARNING("failed creating m_defaultInputLayout.");
        return false;
    }

    std::string shaderByteCodeAnim = m_animationVertexShader.GetShaderByteCode();
    D3D11_INPUT_ELEMENT_DESC animationVertexShaderDesc[] =
    {
        {"POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0,                0,                   D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BINORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BONEIDS",     0, DXGI_FORMAT_R32G32B32A32_UINT,  0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BONEWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    if (FAILED(hr = D3D11Core::Get().Device()->CreateInputLayout(animationVertexShaderDesc, ARRAYSIZE(animationVertexShaderDesc), shaderByteCodeAnim.c_str(), shaderByteCodeAnim.length(), &m_animationInputLayout)))
    {
        LOG_WARNING("failed creating m_animationInputLayout.");
        return false;
    }

    return true;
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
    //b.deltaTime = 0;

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

    std::string shaderByteCodeSky = m_skyboxVertexShader.GetShaderByteCode();
    D3D11_INPUT_ELEMENT_DESC skyboxVertexShaderDesc[1] =
    {
        {"SKYPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    if (FAILED(hr = D3D11Core::Get().Device()->CreateInputLayout(skyboxVertexShaderDesc, 1, shaderByteCodeSky.c_str(), shaderByteCodeSky.length(), &m_skyboxInputLayout)))
    {
        LOG_WARNING("failed creating m_skyboxInputLayout.");
        return false;
    }

    std::string shaderByteCodeParticle = m_ParticleVertexShader.GetShaderByteCode();
    D3D11_INPUT_ELEMENT_DESC particleVertexShaderDesc[] =
    {
        {"POSITION",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,                0,                   D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"VELOCITY",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR",       0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"SIZE",        0, DXGI_FORMAT_R32G32B32_FLOAT,    0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TYPE",        0, DXGI_FORMAT_R32_UINT,           0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"LIFE",        0, DXGI_FORMAT_R32_FLOAT,          0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    if (FAILED(hr = D3D11Core::Get().Device()->CreateInputLayout(particleVertexShaderDesc, ARRAYSIZE(particleVertexShaderDesc), shaderByteCodeParticle.c_str(), shaderByteCodeParticle.length(), &m_ParticleInputLayout)))
    {
        LOG_WARNING("failed creating m_ParticleInputLayout.");
        return false;
    }

    return !FAILED(hr);
}

bool PipelineManager::CreateTextureEffectResources()
{
    /*
    * Get all the textures needed. 
    * Create original resource textures (to reference from and not modify, the modifications happenes on the material that is already on the model)
    * Create URV for the texture that will be modefied.
    * Create SRV for the texture that wont. 
    */

    // Create render textures, target views and shader resource views here
    m_ModdedWaterBlendAlbedoMap = ResourceManager::Get().GetResource<RTexture>("WaterBlendMap.jpg");

    //Get all models needed
    m_WaterModel      = ResourceManager::Get().GetResource<RModel>("WaterMesh.fbx");
    m_WaterEdgeModel  = ResourceManager::Get().GetResource<RModel>("WaterEdgeMesh.fbx");
    m_WaterFloorModel = ResourceManager::Get().GetResource<RModel>("WaterFloorMesh.fbx");


    //Get the textures from the models
    m_ModdedWaterAlbedoMap      = m_WaterModel.get()->GetMeshes()[0].GetTexture(ETextureType::albedo);
    m_ModdedWaterNormalMap      = m_WaterModel.get()->GetMeshes()[0].GetTexture(ETextureType::normal);
    m_ModdedWaterEdgeAlbedoMap  = m_WaterEdgeModel.get()->GetMeshes()[0].GetTexture(ETextureType::albedo);
    m_ModdedWaterFloorAlbedoMap = m_WaterFloorModel.get()->GetMeshes()[0].GetTexture(ETextureType::albedo);

    //Disable mipmaps
    m_ModdedWaterAlbedoMap->DisableMipmaps();
    m_ModdedWaterNormalMap->DisableMipmaps();
    m_ModdedWaterEdgeAlbedoMap->DisableMipmaps();
    m_ModdedWaterFloorAlbedoMap->DisableMipmaps();
    m_ModdedWaterBlendAlbedoMap->DisableMipmaps();

    HRESULT hr = {};

    //  CREATE WATER REFRACTION EFFECT RESOURCES  //
    
        // CREATE TEXTURE 2D //
    // Create texture to keep original resource of water floor //
    const unsigned int textureWidth = 256;
    const unsigned int textureHeight = 256;

    D3D11_TEXTURE2D_DESC desc;
    desc.Width = textureWidth;
    desc.Height = textureHeight;
    desc.MiscFlags = 0;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    //Create original resource reference texture
    hr = m_d3d11->Device()->CreateTexture2D(&desc, nullptr, m_WaterFloorAlbedoMap.GetAddressOf());
    if (FAILED(hr))
        return false;
    
    // Copy Original resource to original resource reference texture. 
    m_d3d11->DeviceContext()->CopyResource(m_WaterFloorAlbedoMap.Get(), m_ModdedWaterFloorAlbedoMap.get()->GetTexture2D());

    // UNORDERED ACCES VIEW //
    hr = m_d3d11->Device()->CreateUnorderedAccessView(m_ModdedWaterFloorAlbedoMap.get()->GetTexture2D(), nullptr, m_UAV_TextureEffectWaterFloorMap.GetAddressOf());
    if (FAILED(hr))
        return false;

    // SHADER RESOURCE VIEW //
    hr = m_d3d11->Device()->CreateShaderResourceView(m_WaterFloorAlbedoMap.Get(), nullptr, m_SRV_TextureEffectWaterFloorMap.GetAddressOf());
    if (FAILED(hr))
        return false;


    //  CREATE WATER EFFECT RESOURCES  //
    /*
    Create texture to keep original resource of water
    It has the same size at the eater floor so use the same desc.
    */

        // ALBEDO MAP //

    //Create original resource reference texture
    hr = m_d3d11->Device()->CreateTexture2D(&desc, nullptr, m_WaterAlbedoMap.GetAddressOf());
    if (FAILED(hr))
        return false;

    // Copy Original resource to original resource reference texture. 
    m_d3d11->DeviceContext()->CopyResource(m_WaterAlbedoMap.Get(), m_ModdedWaterAlbedoMap.get()->GetTexture2D());

    // UNORDERED ACCES VIEW //
    hr = m_d3d11->Device()->CreateUnorderedAccessView(m_ModdedWaterAlbedoMap.get()->GetTexture2D(), nullptr, m_UAV_TextureEffectWaterMap.GetAddressOf());
    if (FAILED(hr))
        return false;

    // SHADER RESOURCE VIEW //
    hr = m_d3d11->Device()->CreateShaderResourceView(m_WaterAlbedoMap.Get(), nullptr, m_SRV_TextureEffectWaterMap.GetAddressOf());
    if (FAILED(hr))
        return false;


        // NORMAL MAP //

    //Create original resource reference texture
    hr = m_d3d11->Device()->CreateTexture2D(&desc, nullptr, &m_WaterNormalMap);
    if (FAILED(hr))
        return false;

    // Copy Original resource to original resource reference texture. 
    m_d3d11->DeviceContext()->CopyResource(m_WaterNormalMap.Get(), m_ModdedWaterNormalMap.get()->GetTexture2D());

    // UNORDERED ACCES VIEW //
    hr = m_d3d11->Device()->CreateUnorderedAccessView(m_ModdedWaterNormalMap.get()->GetTexture2D(), nullptr, m_UAV_TextureEffectWaterNormalMap.GetAddressOf());
    if (FAILED(hr))
        return false;

    // SHADER RESOURCE VIEW //
    hr = m_d3d11->Device()->CreateShaderResourceView(m_WaterNormalMap.Get(), nullptr, m_SRV_TextureEffectWaterNormalMap.GetAddressOf());
    if (FAILED(hr))
        return false;

        // BLEND MAP //
    /*
    * The blendmap need only be read as i'll put the color right on top of the modefied water texture. 
    * A SRV already exists fpr the blendmap provided by the RTexture class. 
    * NVM i need it. 
    */


    //Create original resource reference texture
    hr = m_d3d11->Device()->CreateTexture2D(&desc, nullptr, &m_WaterBlendAlbedoMap);
    if (FAILED(hr))
        return false;

    // Copy Original resource to original resource reference texture. 
    m_d3d11->DeviceContext()->CopyResource(m_WaterBlendAlbedoMap.Get(), m_ModdedWaterBlendAlbedoMap.get()->GetTexture2D());


    // UNORDERED ACCES VIEW //
    hr = m_d3d11->Device()->CreateUnorderedAccessView(m_ModdedWaterBlendAlbedoMap.get()->GetTexture2D(), nullptr, m_UAV_TextureEffectBlendMap.GetAddressOf());
    if (FAILED(hr))
        return false;

    // SHADER RESOURCE VIEW //
    hr = m_d3d11->Device()->CreateShaderResourceView(m_WaterBlendAlbedoMap.Get(), nullptr, m_SRV_TextureEffectBlendMap.GetAddressOf());
    if (FAILED(hr))
        return false;

    return true;
}

bool PipelineManager::CreateDepth()
{
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
    D3D11_TEXTURE2D_DESC textureDesc = {};
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};

    textureDesc.Width = static_cast<FLOAT>(this->m_window->GetWidth());
    textureDesc.Height = static_cast<FLOAT>(this->m_window->GetHeight());
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    HRESULT hr = m_d3d11->Device()->CreateTexture2D(&textureDesc, nullptr, m_depth.texture.GetAddressOf());
    if (FAILED(hr))
        return false;

    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;
    depthStencilViewDesc.Flags = 0;
    depthStencilViewDesc.Texture2D.MipSlice = 0;
    // Create the depth stencil view.
    hr = m_d3d11->Device()->CreateDepthStencilView(m_depth.texture.Get(), &depthStencilViewDesc, m_depth.dsv.GetAddressOf());
    if (FAILED(hr))
        return false;

    shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;

    hr = m_d3d11->Device()->CreateShaderResourceView(m_depth.texture.Get(), &shaderResourceViewDesc, m_depth.srv.GetAddressOf()); 

    return !FAILED(hr);

}

bool PipelineManager::CreateForwardBlendStates()
{
    D3D11_BLEND_DESC blendDescOn;
    ZeroMemory(&blendDescOn, sizeof(D3D11_BLEND_DESC));
    // HERE IS MY GREATEST SUCC
    blendDescOn.AlphaToCoverageEnable = TRUE;
    blendDescOn.IndependentBlendEnable = TRUE;
    blendDescOn.RenderTarget[0].BlendEnable = TRUE;
    blendDescOn.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDescOn.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDescOn.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDescOn.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDescOn.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDescOn.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDescOn.RenderTarget[0].RenderTargetWriteMask = 0x0f;

    D3D11_BLEND_DESC blendDescOff;
    ZeroMemory(&blendDescOff, sizeof(D3D11_BLEND_DESC));

    blendDescOff.RenderTarget[0].BlendEnable = false;
    blendDescOff.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    HRESULT hr = m_d3d11->Device()->CreateBlendState(&blendDescOn, m_blendOn.GetAddressOf());
    if (FAILED(hr))
        return false;

    hr = m_d3d11->Device()->CreateBlendState(&blendDescOff, m_blendOff.GetAddressOf());

    return !FAILED(hr);
}

bool PipelineManager::CreateShaders()
{
    if (!m_defaultVertexShader.Create("Model_vs"))
    {
        LOG_WARNING("failed creating Model_vs.");
        return false;
    }

    if (!m_lightCullingShader.Create("LightCulling_cs"))
    {
        LOG_WARNING("failed creating LightCulling_cs.");
        return false;
    }

    if (!m_depthPassPixelShader.Create("Depth_ps"))
    {
        LOG_WARNING("failed creating Depth_ps.");
        return false;
    }

    if (!m_computeFrustumsShader.Create("ComputeFrustums_cs"))
    {
        LOG_WARNING("failed creating ComputeFrustums_cs.");
        return false;
    }
    
    if (!m_depthPassVertexShader.Create("Depth_vs"))
    {
        LOG_WARNING("failed creating Depth_vs.");
        return false;
    }

    if (!m_animationVertexShader.Create("AnimModel_vs"))
    {
        LOG_WARNING("failed creating AnimModel_vs.");
        return false;
    }

    if (!m_skyboxVertexShader.Create("Skybox_vs"))
    {
        LOG_WARNING("failed creating Skybox_vs.");
        return false;
    }

    if (!m_skyboxPixelShader.Create("Skybox_ps"))
    {
        LOG_WARNING("failed creating Skybox_ps.");
        return false;
    }

    if (!m_defaultPixelShader.Create("Model_ps"))
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

    if (!m_textureEffectVertexShader.Create("textureEffect_vs"))
    {
        LOG_WARNING("failed to create textureEffect_vs");
        return false;
    }
    
    if (!m_WaterEffectComputeShader.Create("waterEffect_cs"))
    {
        LOG_WARNING("failed to create waterEffect_cs");
        return false;
    }

    if (!m_WaterEffectPixelShader.Create("waterEffect_ps"))
    {
        LOG_WARNING("failed to create waterEffect_ps");
        return false;
    }

    if (!m_WaterEffectVertexShader.Create("waterEffect_vs"))
    {
        LOG_WARNING("failed to create waterEffect_vs");
        return false;
    }
    
    if (!m_debugPixelShader.Create("Debug_ps"))
    {
        LOG_WARNING("failed creating Debug_ps.");
        return false;
    }

    if (!m_paraboloidVertexShader.Create("Paraboloid_vs"))
    {
        LOG_WARNING("failed creating Paraboloid_vs.");
        return false;
    }
    
    if (!m_paraboloidAnimationVertexShader.Create("ParaboloidAnim_vs"))
    {
        LOG_WARNING("failed creating ParaboloidAnim_vs.");
        return false;
    }
    
    if (!m_shadowPixelShader.Create("Shadow_ps"))
    {
        LOG_WARNING("failed creating Shadow_ps.");
        return false;
    }

    if (!m_ParticleVertexShader.Create("Particle_vs"))
    {
        LOG_WARNING("failed creating Particle_vs.");
        return false;
    }

    if (!m_ParticlePixelShader.Create("Particle_ps"))
    {
        LOG_WARNING("failed creating Particle_ps.");
        return false;
    }

    if (!m_ParticleGeometryShader.Create("Particle_gs"))
    {
        LOG_WARNING("failed creating Particle_gs.");
        return false;
    }    

    if (!m_ParticleComputeShader.Create("Particle_cs"))
    {
        LOG_WARNING("failed creating Particle_cs.");
        return false;
    }

    if (!m_blurComputeShader.Create("Blur_CS"))
    {
        LOG_WARNING("failed creating Blur_CS");
        return false;
    }

    if (!m_dofComputeShader.Create("DOF_cs"))
    {
        LOG_WARNING("failed creating DOF_cs");
        return false;
    }

    return true;
}
