#include "EnginePCH.h"
#include "Renderer.h"


Renderer::Renderer()
	: pWindow(nullptr)
	, isInitialized(false)
{
}

void Renderer::initialize(Window* pWindow)
{
    assert(isInitialized || "D3D11Core is already initialized.");
    if (this->pWindow == nullptr)
        this->pWindow = pWindow;
	
    // Initialize RenderTargetView.
    if (!this->createRenderTargetView())
		LOG_ERROR("failed creating RenderTargetView.");
    
    // Initialize DepthStencilBuffer.
    if (!this->createDepthStencilTexture())
        LOG_ERROR("failed creating DepthStencilBuffer.");
    
    // Initialize DepthStencilState.
    if (!this->createDepthStencilState())
        LOG_ERROR("failed creating DepthStencilState.");
    
    // Initialize DepthStencilView.
    if (!this->createDepthStencilView())
        LOG_ERROR("failed creating DepthStencilView.");
    
    // Initialize RasterizerStates.
    if (!this->createRasterizerStates())
        LOG_ERROR("failed creating RasterizerStates.");
    
    // Initialize SamplerStates.
    if (!this->createSamplerStates())
        LOG_ERROR("failed creating SamplerStates.");
    
    // Set Viewport.
    this->setViewport();

    this->isInitialized = true;
}

bool Renderer::createRenderTargetView()
{
    ID3D11Texture2D* pBackBuffer = nullptr;

	// Temporary holder D3D11.
    const auto temp = &D3D11Core::Get();
	
    // Get the pointer to the back buffer.
    if (FAILED(temp->SwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pBackBuffer))))
        return false;

    // Create the renderTargetView with the back buffer pointer.
    HRESULT hr = temp->Device()->CreateRenderTargetView(pBackBuffer, nullptr, this->renderTargetView.GetAddressOf());

    // Release pointer to the back buffer.
    pBackBuffer->Release();

    return !FAILED(hr);
}

bool Renderer::createDepthStencilTexture()
{
    // Initialize the description of the depth buffer.
    D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
    ZeroMemory(&depthStencilBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));

    // Set up the description of the depth buffer.
    depthStencilBufferDesc.Width = this->pWindow->GetWidth();
    depthStencilBufferDesc.Height = this->pWindow->GetHeight();
    depthStencilBufferDesc.MipLevels = 1;
    depthStencilBufferDesc.ArraySize = 1;
    depthStencilBufferDesc.SampleDesc.Count = 1;
    depthStencilBufferDesc.SampleDesc.Quality = 0;
    depthStencilBufferDesc.CPUAccessFlags = 0;
    depthStencilBufferDesc.MiscFlags = 0;
    depthStencilBufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilBufferDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    depthStencilBufferDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_DEPTH_STENCIL;

    // Create the texture for the depth buffer using the filled out description.
    HRESULT hr = D3D11Core::Get().Device()->CreateTexture2D(&depthStencilBufferDesc, nullptr, this->depthStencilTexture.GetAddressOf());

    return !FAILED(hr);
}

bool Renderer::createDepthStencilState()
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

    // Create the depth stencil state.
    HRESULT hr = D3D11Core::Get().Device()->CreateDepthStencilState(&depthStencilDesc, this->depthStencilState.GetAddressOf());

    if (SUCCEEDED(hr))
    {
        // Set the default depth stencil state.
        D3D11Core::Get().DeviceContext()->OMSetDepthStencilState(this->depthStencilState.Get(), 1);
    }
    return !FAILED(hr);
}

bool Renderer::createDepthStencilView()
{
    // Initialize the depth stencil View.
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

    // Set up the depth stencil View description, setting applied for Deferred Rendering.
    depthStencilViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // Create the depth stencil View.
    HRESULT hr = D3D11Core::Get().Device()->CreateDepthStencilView(this->depthStencilTexture.Get(), &depthStencilViewDesc, this->depthStencilView.GetAddressOf());

    if (SUCCEEDED(hr))
    {
        // Bind the render target View and depth stencil buffer to the output render pipeline.
        D3D11Core::Get().DeviceContext()->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), this->depthStencilView.Get());
    }

    return !FAILED(hr);
}

bool Renderer::createRasterizerStates()
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

    // Temporary D3D11 holder.
    const auto temp = &D3D11Core::Get();
	
    // Create the rasterizer state from the description we just filled out.
    HRESULT hr = temp->Device()->CreateRasterizerState(&rasterizerDesc, this->rasterizerState.GetAddressOf());

    // Setup a raster description with no back face culling.
    rasterizerDesc.CullMode = D3D11_CULL_NONE;

    // Create the no culling rasterizer state.
    hr = temp->Device()->CreateRasterizerState(&rasterizerDesc, this->rasterStateNoCulling.GetAddressOf());
    if (FAILED(hr))
        return false;

    // Setup a raster description which enables wire frame rendering.
    rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
    rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;

    // Create the wire frame rasterizer state.
    hr = temp->Device()->CreateRasterizerState(&rasterizerDesc, this->rasterStateWireframe.GetAddressOf());

    // Set default rasterizer state.
    temp->DeviceContext()->RSSetState(this->rasterStateNoCulling.Get());

    return !FAILED(hr);
}

bool Renderer::createSamplerStates()
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

    HRESULT hr = D3D11Core::Get().Device()->CreateSamplerState(&samplerDesc, this->linearSamplerState.GetAddressOf());
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

    hr = D3D11Core::Get().Device()->CreateSamplerState(&samplerDesc, this->pointSamplerState.GetAddressOf());

    return !FAILED(hr);
}

void Renderer::setViewport()
{
    // Initialize the viewport to occupy the entire client area.
    RECT clientRect;
    GetClientRect(this->pWindow->GetHWnd(), &clientRect);
    const LONG clientWidth = clientRect.right - clientRect.left;
    const LONG clientHeight = clientRect.bottom - clientRect.top;

    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    // Setup the viewport for rendering.
    viewport.Width = static_cast<FLOAT>(this->pWindow->GetWidth());
    viewport.Height = static_cast<FLOAT>(this->pWindow->GetHeight());
    viewport.TopLeftX = 0.f;
    viewport.TopLeftY = 0.f;

    // Direct3D uses a depth buffer range of 0 to 1, hence:
    viewport.MinDepth = 0.f;
    viewport.MaxDepth = 1.f;

    // Set viewport.
    D3D11Core::Get().DeviceContext()->RSSetViewports(1, &this->viewport);
}


