#pragma once
#include "Server.h"

LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


class Window
{
public:
    // Window description with default values.
    // Use when initializing the m_window.
    struct Desc
    {
        unsigned int width = static_cast<unsigned int>(GetSystemMetrics(SM_CXSCREEN) / 2);
        unsigned int height = static_cast<unsigned int>(GetSystemMetrics(SM_CYSCREEN) / 2);
        LPCWSTR title = L"Default";
        bool fullScreen = false;
        bool resizableWindow = false;
        HINSTANCE hInstance = nullptr;
        int nShowCmd = SW_HIDE;
    };

private:
    HWND m_hWnd;
    RECT m_clientRect;
    Desc m_windowDesc;

public:
    Window();
    Window(const Window& other) = delete;
    Window(Window&& other) = delete;
    Window& operator=(const Window& other) = delete;
    Window& operator=(Window&& other) = delete;
    virtual ~Window();

    bool Initialize(const Desc & desc = Desc());
    bool IsFullScreen() const;
    void SetFullScreen(bool fullscreen);

    HWND GetHWnd() const;
    RECT GetClientRect() const;
    unsigned int GetWidth() const;
    unsigned int GetHeight() const;

};


