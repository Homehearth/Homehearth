#include "EnginePCH.h"
#include "Renderer.h"


Renderer::Renderer()
	: m_window(nullptr)
	, m_d3d11(nullptr)
{
}

void Renderer::Initialize(Window* pWindow)
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
    if (!this->CreateDepthStencilState())
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

    this->CreateDefaultShaders();
        //LOG_ERROR("failed creating default shaders.");

    this->CreateDefaultLayout();
        //LOG_ERROR("failed creating default layout.");

    this->CreateDefaultcBuffer();
        //LOG_ERROR("failed creating default cBuffer.");
	
    // Set Viewport.
    this->SetViewport();

}

bool Renderer::CreateRenderTargetView()
{
    ID3D11Texture2D* pBackBuffer = nullptr;

    // Get the pointer to the back buffer.
    if (FAILED(m_d3d11->SwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pBackBuffer))))
        return false;

    // Create the renderTargetView with the back buffer pointer.
    HRESULT hr = m_d3d11->Device()->CreateRenderTargetView(pBackBuffer, nullptr, this->renderTargetView.GetAddressOf());

    // Release pointer to the back buffer.
    pBackBuffer->Release();

    return !FAILED(hr);
}

bool Renderer::CreateDepthStencilTexture()
{
    // Initialize the description of the depth buffer.
    D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
    ZeroMemory(&depthStencilBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));

    // Set up the description of the depth buffer.
    depthStencilBufferDesc.Width = this->m_window->GetWidth();
    depthStencilBufferDesc.Height = this->m_window->GetHeight();
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
    HRESULT hr = m_d3d11->Device()->CreateTexture2D(&depthStencilBufferDesc, nullptr, this->depthStencilTexture.GetAddressOf());

    return !FAILED(hr);
}

bool Renderer::CreateDepthStencilState()
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
    HRESULT hr = m_d3d11->Device()->CreateDepthStencilState(&depthStencilDesc, this->depthStencilState.GetAddressOf());

    if (SUCCEEDED(hr))
    {
        // Set the default depth stencil state.
        m_d3d11->DeviceContext()->OMSetDepthStencilState(this->depthStencilState.Get(), 1);
    }
    return !FAILED(hr);
}

bool Renderer::CreateDepthStencilView()
{
    // Initialize the depth stencil View.
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

    // Set up the depth stencil View description, setting applied for Deferred Rendering.
    depthStencilViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION::D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // Create the depth stencil View.
    HRESULT hr = m_d3d11->Device()->CreateDepthStencilView(this->depthStencilTexture.Get(), &depthStencilViewDesc, this->depthStencilView.GetAddressOf());

    if (SUCCEEDED(hr))
    {
        // Bind the render target View and depth stencil buffer to the output render pipeline.
        m_d3d11->DeviceContext()->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), this->depthStencilView.Get());
    }

    return !FAILED(hr);
}

bool Renderer::CreateRasterizerStates()
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
    HRESULT hr = m_d3d11->Device()->CreateRasterizerState(&rasterizerDesc, this->rasterizerState.GetAddressOf());

    // Setup a raster description with no back face culling.
    rasterizerDesc.CullMode = D3D11_CULL_NONE;

    // Create the no culling rasterizer state.
    hr = m_d3d11->Device()->CreateRasterizerState(&rasterizerDesc, this->rasterStateNoCulling.GetAddressOf());
    if (FAILED(hr))
        return false;

    // Setup a raster description which enables wire frame rendering.
    rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
    rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;

    // Create the wire frame rasterizer state.
    hr = m_d3d11->Device()->CreateRasterizerState(&rasterizerDesc, this->rasterStateWireframe.GetAddressOf());

    // Set default rasterizer state.
    m_d3d11->DeviceContext()->RSSetState(this->rasterStateNoCulling.Get());

    return !FAILED(hr);
}

bool Renderer::CreateSamplerStates()
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

void Renderer::SetViewport()
{
    // Initialize the viewport to occupy the entire client area.
    RECT clientRect;
    GetClientRect(this->m_window->GetHWnd(), &clientRect);
    const LONG clientWidth = clientRect.right - clientRect.left;
    const LONG clientHeight = clientRect.bottom - clientRect.top;

    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    // Setup the viewport for rendering.
    viewport.Width = static_cast<FLOAT>(this->m_window->GetWidth());
    viewport.Height = static_cast<FLOAT>(this->m_window->GetHeight());
    viewport.TopLeftX = 0.f;
    viewport.TopLeftY = 0.f;

    // Direct3D uses a depth buffer range of 0 to 1, hence:
    viewport.MinDepth = 0.f;
    viewport.MaxDepth = 1.f;

    // Set viewport.
    m_d3d11->DeviceContext()->RSSetViewports(1, &this->viewport);
}

void Renderer::SetPipelineState()
{
#define CONTEXT D3D11Core::Get().DeviceContext()
	/*
	 * 

	// STRIDE & OFFSET.
    static UINT stride = sizeof(DefaultVertexBuffer);
    static UINT offset = 0;

    const auto dc = m_d3d11->DeviceContext();
	
    // INPUT ASSEMBLY.
    {
        dc->IASetInputLayout();
        dc->IASetPrimitiveTopology();
    }

    // SHADER STAGES.
    {
        dc->VSSetShader();
        dc->PSSetShader();
        dc->GSSetShader();
        dc->HSSetShader();
        dc->DSSetShader();
        dc->CSSetShader();
    }

    // CONSTANT BUFFERS.
    {
        dc->PSSetConstantBuffers();
        dc->VSSetConstantBuffers();
    }

    // SHADER RESOURCES.
    {    
		dc->PSSetShaderResources();
		dc->VSSetShaderResources();
		dc->PSSetSamplers();
    }

    // RASTERIZER.
    {
        dc->RSSetViewports();
        dc->RSSetState();
    }

    // OUTPUT MERGER.
    {
        dc->OMSetRenderTargets();
        dc->OMSetBlendState();
        dc->OMSetDepthStencilState();
    }
	 */


    /*
        Absolute default rendering.
    */
    CONTEXT->PSSetSamplers(0, 1, linearSamplerState.GetAddressOf());
    CONTEXT->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    //CONTEXT->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());
    CONTEXT->VSSetConstantBuffers(0, 1, m_defaultcBuffer.GetAddressOf());
    CONTEXT->IASetInputLayout(m_defaultLayout.Get());
    CONTEXT->VSSetShader(m_defaultVertexShader.Get(), nullptr, NULL);
    CONTEXT->PSSetShader(m_defaultPixelShader.Get(), nullptr, NULL);
}

bool Renderer::CreateDefaultLayout()
{
    D3D11_INPUT_ELEMENT_DESC inputDesc[] =
    {
        {"POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0,                0,                    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,       0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",        0, DXGI_FORMAT_R32G32B32_FLOAT,    0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT",        0, DXGI_FORMAT_R32G32B32_FLOAT,    0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BINORMAL",        0, DXGI_FORMAT_R32G32B32_FLOAT,    0,    D3D11_APPEND_ALIGNED_ELEMENT,    D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    HRESULT hr = D3D11Core::Get().Device()->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), shaderByteCode.c_str(), shaderByteCode.length(), &m_defaultLayout);

    return !FAILED(hr);
}

bool Renderer::CreateDefaultcBuffer()
{
    D3D11_BUFFER_DESC bDesc;
    bDesc.ByteWidth = sizeof(BasicModelMatrix);
    bDesc.Usage = D3D11_USAGE_DEFAULT;
    bDesc.CPUAccessFlags = 0;
    bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bDesc.MiscFlags = 0;

    BasicModelMatrix b;
    b.worldMatrix = sm::Matrix::CreateWorld({ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f });

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = &(b.worldMatrix);

    HRESULT hr = D3D11Core::Get().Device()->CreateBuffer(&bDesc, &data, m_defaultcBuffer.GetAddressOf());

    return !FAILED(hr);
}

bool Renderer::CreateDefaultShaders()
{

    std::ifstream reader;
    std::string shaderData;
    reader.open("Model_vs.cso", std::ios::binary | std::ios::ate);
    if (!reader.is_open())
    {
        std::cout << "ERROR::loadShaderData()::Could not open Model_vs.cso" << std::endl;
        return false;
    }
    reader.seekg(0, std::ios::end);
    shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
    reader.seekg(0, std::ios::beg);
    shaderData.assign((std::istreambuf_iterator<char>(reader)), std::istreambuf_iterator<char>());
    shaderByteCode = shaderData;
    reader.close();

    HRESULT hr = D3D11Core::Get().Device()->CreateVertexShader(shaderByteCode.c_str(), shaderByteCode.length(), nullptr, m_defaultVertexShader.GetAddressOf());

    std::ifstream reader2;
    std::string shaderData2;
    reader2.open("Model_ps.cso", std::ios::binary | std::ios::ate);
    if (!reader2.is_open())
    {
        std::cout << "ERROR::loadShaderData()::Could not open Model_ps.cso" << std::endl;
        return false;
    }
    reader2.seekg(0, std::ios::end);
    shaderData2.reserve(static_cast<unsigned int>(reader2.tellg()));
    reader2.seekg(0, std::ios::beg);
    shaderData2.assign((std::istreambuf_iterator<char>(reader2)), std::istreambuf_iterator<char>());
    reader2.close();

    hr = D3D11Core::Get().Device()->CreatePixelShader(shaderData2.c_str(), shaderData2.length(), nullptr, m_defaultPixelShader.GetAddressOf());
   
    return !FAILED(hr);
}


