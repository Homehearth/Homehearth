#pragma once

/*
 *	Initialize D3D11: device, device-context, swapchain.
 */

class D3D11Core
{
public:
	D3D11Core(const D3D11Core& other) = delete;
	D3D11Core(D3D11Core&& other) = delete;
	D3D11Core& operator=(const D3D11Core& other) = delete;
	D3D11Core& operator=(D3D11Core&& other) = delete;
	virtual ~D3D11Core() = default;

	static auto& Get(){
		static D3D11Core instance;
		return instance;
	}

private:
	D3D11Core() = default;

	ComPtr<ID3D11Device>			device;
	ComPtr<ID3D11DeviceContext>		deviceContext;
	ComPtr<IDXGISwapChain>			swapChain;
	DXGI_FORMAT						swapChainFormat;
	D3D11_VIEWPORT					viewport;
};

