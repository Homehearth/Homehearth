#include "EnginePCH.h"
#include "D3D11Core.h"

D3D11Core::D3D11Core()
	: pWindow(nullptr)
    , isInitialized(false)
{
}

void D3D11Core::initialize(Window * pWindow)
{
    assert(!isInitialized && "D3D11Core is already initialized.");
    if (this->pWindow == nullptr)
        this->pWindow = pWindow;
	
    // Initialize DeviceAndSwapChain.
    if (!this->createDeviceAndSwapChain())
        LOG_ERROR("failed creating device and swapchain.");

	
	/*
    // Initialize RenderTargetView.
    if (!this->createRenderTargetView())
	
    // Initialize DepthStencilBuffer.
    if (!this->createDepthStencilTexture())
	
    // Initialize DepthStencilState.
    if (!this->createDepthStencilState())
	
    // Initialize DepthStencilView.
    if (!this->createDepthStencilView())
	
    // Initialize RasterizerStates.
    if (!this->createRasterizerStates())
	
    // Initialize SamplerStates.
    if (!this->createSamplerStates())
	
    // Set Viewport.
    this->setViewport();    
	 */
}

bool D3D11Core::createDeviceAndSwapChain()
{
    // Initialize the swap chain description.
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

    // Set the handle for the window to render to.
    swapChainDesc.OutputWindow = pWindow->getHWnd();

    // Set to windowed mode.
    swapChainDesc.Windowed = pWindow->isFullScreen() ? FALSE : TRUE;
    
    // Set the client dimensions of the back buffer.
    swapChainDesc.BufferDesc.Width = this->pWindow->getWidth();
    swapChainDesc.BufferDesc.Height = this->pWindow->getHeight();

    // Set to a duo back buffer.
    swapChainDesc.BufferCount = 2;

    // Set the refresh rate of the back buffer.
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

    // Set regular 32-bit surface for the back buffer.
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    // Set the scan line ordering and scaling to unspecified.                             
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // Set the usage of the back buffer.
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS | DXGI_USAGE_SHADER_INPUT;

    // Turn multisampling off.
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

    // Discard the contents of the back buffer after IDXGISwapChain1::Present() call.
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    // Set this flag to enable an application to switch modes by calling IDXGISwapChain::ResizeTarget.
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    // Creates a device that supports the debug layer. 
    UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	
    // Set the feature level to dx 11.
    D3D_FEATURE_LEVEL featureLevels[]
    {
        D3D_FEATURE_LEVEL_11_0
    };

    // Create the device, swap chain and device context.
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, _countof(featureLevels), D3D11_SDK_VERSION,
        &swapChainDesc, this->swapChain.GetAddressOf(), this->device.GetAddressOf(), nullptr, this->deviceContext.GetAddressOf());

    return !FAILED(hr);
}