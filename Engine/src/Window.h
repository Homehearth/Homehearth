#pragma once

LRESULT CALLBACK WinProc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam);

class Window
{
public:
    Window();
    virtual ~Window(){ DestroyWindow(this->hWnd); };

    // Window description with default values.
    // Use when initializing the window.
    struct Desc
    {
        INT width = GetSystemMetrics(SM_CXSCREEN) / 2;
        INT height = GetSystemMetrics(SM_CYSCREEN) / 2;
        LPCWSTR title = L"Default";
        LPCWSTR windowClass = L"Window Class A";
        BOOL fullScreen = false;
        BOOL resizableWindow = false;
        HINSTANCE hInstance = nullptr;
        INT nShowCmd = SW_NORMAL;
    };

    // Initialize window.
    BOOL initialize(const Desc & desc = Desc());

	// Switch for window- / fullscreen mode.
    VOID fullScreenSwitch();
   	
    HWND getHWnd() const;
    RECT getClientRect() const;
    INT getWidth() const;
    INT getHeight() const;

private:
    HWND hWnd;
    RECT clientRect;
    Desc windowDesc;
};

