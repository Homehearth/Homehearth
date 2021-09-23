#include "NetServerPCH.h"
#include "Window.h"
#include "InputSystemServer.h"

LRESULT CALLBACK Window::WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Engine events:
	switch (uMsg)
	{
	case WM_NCCREATE:
		LOG_INFO("Window has been created.");
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		// https://docs.microsoft.com/en-us/windows/win32/direct3ddxgi/d3d10-graphics-programming-guide-dxgi#handling-window-resizing
		break;
	default:
		break;
	}

	// Application events:
	if (uMsg == WM_KEYDOWN ||
		uMsg == WM_KEYUP ||
		uMsg == WM_CHAR ||
		uMsg == WM_INPUT ||
		uMsg == WM_MBUTTONUP ||
		uMsg == WM_MBUTTONDOWN ||
		uMsg == WM_RBUTTONDOWN ||
		uMsg == WM_RBUTTONUP ||
		uMsg == WM_LBUTTONDBLCLK ||
		uMsg == WM_RBUTTONDBLCLK ||
		uMsg == WM_MBUTTONDBLCLK ||
		uMsg == WM_MOUSEWHEEL ||
		uMsg == WM_MOUSEMOVE)
	{
		// Add the event to the m_eventQueue.
		InputSystemServer::Get().RegisterEvent(uMsg, wParam);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


Window::Window()
	: m_hWnd(nullptr)
	, m_clientRect({})
	, m_windowDesc({})
{
	
}

Window::~Window()
{
	DestroyWindow(this->m_hWnd);
	LOG_INFO("Window has been destroyed.");
}

bool Window::Initialize(const Desc& desc)
{
	// Define window style.
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));

	const LPCWSTR WINDOW_CLASS = L"Default Window Class";
	
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.lpszClassName = WINDOW_CLASS;
	wcex.lpszMenuName = nullptr;
	wcex.hbrBackground = static_cast<HBRUSH>(CreateSolidBrush(RGB(100, 149, 237)));	// Cornflower blue.
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);;
	wcex.hInstance = desc.hInstance;
	wcex.lpfnWndProc = WinProc;

	// Register window class.
	const ATOM result = RegisterClassEx(&wcex);
	assert(result && "Failed to register window class.");

	// Retrieve desktop window.
	RECT desktop;
	const HWND hwndDesktop = GetDesktopWindow();
	GetWindowRect(hwndDesktop, &desktop);

	// Create the window.
	this->m_hWnd = CreateWindowEx(0, WINDOW_CLASS, desc.title,
		WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
		((desktop.right / 2) - (desc.width / 2)),
		((desktop.bottom / 2) - (desc.height / 2)),
		desc.width, desc.height,
		nullptr, nullptr, desc.hInstance, nullptr);

	assert(this->m_hWnd && "Window wasn't successfully created.");

	UpdateWindow(this->m_hWnd);
	ShowWindow(this->m_hWnd, desc.nShowCmd);

	this->m_windowDesc = desc;

	return true;
}

HWND Window::GetHWnd() const
{
	return this->m_hWnd;
}

RECT Window::GetClientRect() const
{
	return this->m_clientRect;
}

unsigned int Window::GetWidth() const
{
	return this->m_windowDesc.width;
}

unsigned int Window::GetHeight() const
{
	return this->m_windowDesc.height;
}

bool Window::IsFullScreen() const
{
	return this->m_windowDesc.fullScreen;
}

void Window::SetFullScreen(bool fullscreen)
{
	assert(this->m_hWnd && "There's no window to resize.");

	// TODO: previous code caused issues.
	//D3D11Core::Get().SwapChain()->SetFullscreenState(true, nullptr);
}
