#pragma once

#include "Window.h"

/*
 *	Initialize D3D11: device, device-context, swapchain.
 */

class D3D11Core
{
public:
	D3D11Core();
	D3D11Core(const D3D11Core& other) = delete;
	D3D11Core(D3D11Core&& other) = delete;
	D3D11Core& operator=(const D3D11Core& other) = delete;
	D3D11Core& operator=(D3D11Core&& other) = delete;
	virtual ~D3D11Core() = default;

	void initialize(Window * pWindow);
private:
	Window* pWindow;
	bool isInitialized;
	
	ComPtr<ID3D11Device>			device;
	ComPtr<ID3D11DeviceContext>		deviceContext;
	ComPtr<IDXGISwapChain>			swapChain;
	D3D11_VIEWPORT					viewport;

	bool createDeviceAndSwapChain();

	/*
	bool createRenderTargetView();
	bool createDepthStencilTexture();
	bool createDepthStencilState();
	bool createDepthStencilView();
	bool createRasterizerStates();
	bool createSamplerStates();
	bool createUnorderedAccesView();
	void setViewport();
	 */
};

