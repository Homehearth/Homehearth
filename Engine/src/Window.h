#pragma once

LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// TODO: cleanup.
class Window
{
public:
    Window();
    Window(const Window& other) = delete;
    Window(Window&& other) = delete;
    Window& operator=(const Window& other) = delete;
    Window& operator=(Window&& other) = delete;
    virtual ~Window();

	
    // Window description with default values.
    // Use when initializing the window.
    struct Desc
    {
        unsigned int width = (unsigned int)GetSystemMetrics(SM_CXSCREEN) / 2;
        unsigned int height = (unsigned int)GetSystemMetrics(SM_CYSCREEN) / 2;
        LPCWSTR title = L"Default";
        bool fullScreen = false;
        bool resizableWindow = false;
        HINSTANCE hInstance = nullptr;
        int nShowCmd = SW_NORMAL;
    };

    bool initialize(const Desc & desc = Desc());
    bool isFullScreen() const;
    void setFullScreen(bool fullscreen);
	
    HWND getHWnd() const;
    RECT getClientRect() const;
    unsigned int getWidth() const;
    unsigned int getHeight() const;

private:	
    HWND hWnd;
    RECT clientRect;
    Desc windowDesc;
};


