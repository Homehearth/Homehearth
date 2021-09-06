#include "EnginePCH.h"
#include "D2D1Core.h"

#define INSTANCE D2D1Core::instance
D2D1Core* INSTANCE = nullptr;

D2D1Core::D2D1Core()
{
	m_writeFactory = nullptr;
	m_factory = nullptr;
	m_writeFont = nullptr;
	m_writeFormat = nullptr;
	m_renderTarget = nullptr;
	m_surface = nullptr;
	m_hwndTarget = nullptr;
	m_solidBrush = nullptr;
}

D2D1Core::~D2D1Core()
{
	if (m_writeFactory)
		m_writeFactory->Release();
	if (m_factory)
		m_factory->Release();
	if (m_writeFont)
		m_writeFont->Release();
	if (m_writeFormat)
		m_writeFormat->Release();
	if (m_renderTarget)
		m_renderTarget->Release();
	if (m_surface)
		m_surface->Release();
	if (m_hwndTarget)
		m_hwndTarget->Release();
	if (m_solidBrush)
		m_solidBrush->Release();
}

const bool D2D1Core::Setup(Window* window)
{
	// Create a factory for D2D1, if it fails we LOG_ERROR and return false.
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &m_factory);
	if (FAILED(hr))
		[] {LOG_ERROR("Creating D2D1Factory failed."); return false; };

	// Get the surface from the backbuffer from D3D11.
	D3D11Core::Get().SwapChain()->GetBuffer(0, IID_PPV_ARGS(&m_surface));

	FLOAT dpi = GetDpiForWindow(window->GetHWnd());

	return true;
}

void D2D1Core::Initialize(Window* window)
{
	if (!INSTANCE)
	{
		INSTANCE = new D2D1Core();
		INSTANCE->Setup(window);
	}
}

void D2D1Core::Destroy()
{
	if (INSTANCE)
		delete INSTANCE;
}
