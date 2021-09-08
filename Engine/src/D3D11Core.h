#pragma once

#include "Window.h"

#include <d3d11_4.h>
#include <dxgi1_6.h>

#include <psapi.h>

#include <string>


class D3D11Core
{
private:
	D3D11Core();

	Window* m_window;
	bool m_isInitialized;

	ComPtr<ID3D11Device>		m_device;
	ComPtr<ID3D11DeviceContext>	m_deviceContext;
	ComPtr<IDXGISwapChain>		m_swapChain;

	// TODO:
	ComPtr<IDXGIDevice>			m_dxgiDevice;	// 1 2 
	ComPtr<IDXGIAdapter>		m_dxgiAdapter;	// 1 2
	ComPtr<IDXGIAdapter4>		m_dxgiAdapter4;	// 1 2
	ComPtr<IDXGIFactory>		m_dxgiFacory;	// 1 2

	bool createDeviceAndSwapChain();

public:
	virtual ~D3D11Core() = default;

	static auto& Get()
	{
		static D3D11Core s_instance;
		return s_instance;
	}
	
	void Initialize(Window * pWindow);

	ID3D11Device* Device() const;
	ID3D11DeviceContext* DeviceContext() const;
	IDXGISwapChain* SwapChain() const;


	DXGI_QUERY_VIDEO_MEMORY_INFO GetVideoMemoryInfo();

	// No copying.
	D3D11Core(const D3D11Core& other) = delete;
	D3D11Core& operator=(const D3D11Core& other) = delete;

};

