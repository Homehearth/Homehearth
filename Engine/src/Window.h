#pragma once
LRESULT CALLBACK WinProc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam);
class Window
{
public:
    Window();
    virtual ~Window() = default;

    // Window description with default values.
    // Use when initializing the window.
    struct Desc
    {
        int width = GetSystemMetrics(SM_CXSCREEN);
        int height = GetSystemMetrics(SM_CYSCREEN);
        LPCWSTR title = L"Default";
        LPCWSTR windowClass = L"Window Class A";
        bool fullScreen = false;
        bool resizableWindow = false;
        HINSTANCE hInstance = nullptr;
        int nShowCmd = -1;
    };

    // Initialize window.
    bool initialize(const Desc & desc = Desc());
    HWND getHWnd() const;
    RECT getClientRect() const;
    int getWidth() const;
    int getHeight() const;

private:
    HWND hWnd;
    RECT clientRect;
    Desc windowDesc;
};

