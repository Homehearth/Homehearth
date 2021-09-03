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
		switch (wparam)	{
			case 0x1B: // Process escape key. 
				PostQuitMessage(0);
				break;
			default:
				break;
		}
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
	RegisterClassEx(&wcex);

	// Retrieve the desktop window.
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