#include "EnginePCH.h"
#include "Window.h"


LRESULT CALLBACK WinProc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	case WM_ERASEBKGND:
	case WM_SIZE:
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYUP:
	case WM_MBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_MOUSEWHEEL:
	case WM_MOUSEMOVE:
	case WM_INPUT:
	case WM_CHAR:
	{
		if (wparam == 0x1B)	// Process escape key. 
			PostQuitMessage(0);
	}
	break;
	default:
		break;
	}

	return DefWindowProc(handle, msg, wparam, lparam);
}


Window::Window()
	: hWnd(nullptr)
	, clientRect({})
	, windowDesc({})
{
}

BOOL Window::initialize(const Desc& desc)
{
	// Define window style.
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.lpszClassName = desc.windowClass;
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
	this->hWnd = CreateWindowEx(0, desc.windowClass, desc.title,
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

INT Window::getWidth() const
{
	return this->windowDesc.width;
}

INT Window::getHeight() const
{
	return this->windowDesc.height;
}

VOID Window::fullScreenSwitch()
{
	assert(this->hWnd && "There's no window to resize.");
	
	WINDOWPLACEMENT wpc = { sizeof(wpc) };
	LONG HWNDStyle = 0;
	LONG HWNDStyleEx = 0;
	
	if (!this->windowDesc.fullScreen)
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
	else
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