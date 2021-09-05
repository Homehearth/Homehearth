#include "EnginePCH.h"
#include "Window.h"
#include "InputSystem.h"

LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Engine Input:
	switch (uMsg)
	{
	case WM_NCCREATE:
	{
		LOG_INFO("Window has been created.");
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	// Application Input:
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
		InputSystem::Get().addEvent(uMsg, wParam);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


Window::Window()
	: hWnd(nullptr)
	, clientRect({})
	, windowDesc({})
{
}

bool Window::initialize(const Desc& desc)
{
	// Define window style.
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));

	const LPCWSTR WINDOW_CLASS = L"DefaultWindowClass";
	
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.lpszClassName = WINDOW_CLASS;
	wcex.lpszMenuName = nullptr;
	wcex.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(100, 149, 237));	// Cornflower blue.
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
	this->hWnd = CreateWindowEx(0, WINDOW_CLASS, desc.title,
		WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
		((desktop.right / 2) - (desc.width / 2)),
		((desktop.bottom / 2) - (desc.height / 2)),
		desc.width, desc.height,
		nullptr, nullptr, desc.hInstance, nullptr);

	assert(this->hWnd && "Window wasn't successfully created.");
	
	UpdateWindow(this->hWnd);
	ShowWindow(this->hWnd, desc.nShowCmd);

	this->windowDesc = desc;

	return true;
}

HWND Window::getHWnd() const
{
	return this->hWnd;
}

RECT Window::getClientRect() const
{
	return this->clientRect;
}

int Window::getWidth() const
{
	return this->windowDesc.width;
}

int Window::getHeight() const
{
	return this->windowDesc.height;
}

bool Window::isFullScreen() const
{
	return this->windowDesc.fullScreen;
}

void Window::setFullScreen(bool fullscreen)
{
	assert(this->hWnd && "There's no window to resize.");

	WINDOWPLACEMENT wpc = { sizeof(wpc) };
	LONG HWNDStyle = 0;
	LONG HWNDStyleEx = 0;

	if (fullscreen && !this->windowDesc.fullScreen)
	{
		this->windowDesc.fullScreen = true;

		GetWindowPlacement(this->hWnd, &wpc);
		if (HWNDStyle == 0)
			HWNDStyle = GetWindowLong(this->hWnd, GWL_STYLE);
		if (HWNDStyleEx == 0)
			HWNDStyleEx = GetWindowLong(this->hWnd, GWL_EXSTYLE);

		LONG NewHWNDStyle = HWNDStyle;
		NewHWNDStyle &= ~WS_BORDER;
		NewHWNDStyle &= ~WS_DLGFRAME;
		NewHWNDStyle &= ~WS_THICKFRAME;

		LONG NewHWNDStyleEx = HWNDStyleEx;
		NewHWNDStyleEx &= ~WS_EX_WINDOWEDGE;

		SetWindowLong(this->hWnd, GWL_STYLE, NewHWNDStyle | WS_POPUP);
		SetWindowLong(this->hWnd, GWL_EXSTYLE, NewHWNDStyleEx | WS_EX_TOPMOST);
		ShowWindow(this->hWnd, SW_SHOWMAXIMIZED);
	}
	else if (!fullscreen && this->windowDesc.fullScreen)
	{
		this->windowDesc.fullScreen = false;

		GetWindowPlacement(this->hWnd, &wpc);
		HWNDStyle = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
		SetWindowLong(this->hWnd, GWL_STYLE, HWNDStyle);
		SetWindowLong(this->hWnd, GWL_EXSTYLE, HWNDStyleEx);
		SetWindowPlacement(this->hWnd, &wpc);
		ShowWindow(this->hWnd, SW_SHOWNORMAL);
	}
}
