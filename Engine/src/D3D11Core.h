#pragma once

#include "Window.h"

class D3D11Core
{
public:
	virtual ~D3D11Core() = default;

	static auto& Get(){
		static D3D11Core instance;
		return instance;
	}
	
	void initialize(Window * pWindow);

	// Getters.
	ID3D11Device* Device() const;
	ID3D11DeviceContext* DeviceContext() const;
	IDXGISwapChain* SwapChain() const;

	// no copying.
	D3D11Core(const D3D11Core& other) = delete;
	D3D11Core& operator=(const D3D11Core& other) = delete;
private:
	D3D11Core();
	
	Window* pWindow;
	bool isInitialized;

	ComPtr<ID3D11Device>		device;
	ComPtr<ID3D11DeviceContext>	deviceContext;
	ComPtr<IDXGISwapChain>		swapChain;

	bool createDeviceAndSwapChain();
};

