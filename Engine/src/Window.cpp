#include "EnginePCH.h"
#include "Window.h"



//--------------------------------------------------------------------------------------
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
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(handle, &ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
		EndPaint(handle, &ps);
	}
	break;
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
		switch (wparam)
		{
			case 0x1B: // Process escape key. 
				PostQuitMessage(0);
				break;
			default:
				break;
		}
	}

	default:
		break;
	}

	return DefWindowProc(handle, msg, wparam, lparam);
}






//--------------------------------------------------------------------------------------
Window::Window()
	: hWnd(nullptr)
	, clientRect({})
	, windowDesc({})
{
}






//--------------------------------------------------------------------------------------
bool Window::initialize(const Desc& desc)
{
	// Define window style.
	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(WNDCLASS));
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = WinProc;
	wc.hInstance = desc.hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = desc.windowClass;
	wc.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(100, 149, 237)));	// Cornflower blue.
	RegisterClass(&wc);

	// Retrieve the desktop window.
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);

	// Create the window
	this->hWnd = CreateWindowEx(0, desc.windowClass, desc.title,
		WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
		((desktop.right / 2) - (desc.width / 2)),
		((desktop.bottom / 2) - (desc.height / 2)),
		desc.width, desc.height,
		nullptr, nullptr, desc.hInstance, nullptr);

	UpdateWindow(this->hWnd);
	ShowWindow(this->hWnd, desc.nShowCmd);

	this->windowDesc = desc;

	return true;
}






//--------------------------------------------------------------------------------------
HWND Window::getHWnd() const
{
	return this->hWnd;
}






//--------------------------------------------------------------------------------------
RECT Window::getClientRect() const
{
	return this->clientRect;
}






//--------------------------------------------------------------------------------------
int Window::getWidth() const
{
	return this->windowDesc.width;
}






//--------------------------------------------------------------------------------------
int Window::getHeight() const
{
	return this->windowDesc.height;
}