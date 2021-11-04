#pragma once
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
class Window
{
public:
    // Window description with default values.
    // Use when initializing m_window.
    struct Desc
    {
        unsigned int width = static_cast<unsigned int>(GetSystemMetrics(SM_CXSCREEN) / 2);
        unsigned int height = static_cast<unsigned int>(GetSystemMetrics(SM_CYSCREEN) / 2);
        LPCWSTR title = L"Default";
        bool fullScreen = false;
        bool resizableWindow = false;
        HINSTANCE hInstance = nullptr;
        int nShowCmd = SW_NORMAL;
    };

private:
    static LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND m_hWnd;
    RECT m_clientRect;
    Desc m_windowDesc;
    
    static void ConfineCursor(HWND hwnd);

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
    void SetWindowTitle(const LPCWSTR &title);
    void SetWindowTextBar(const std::string& text);
	
    HWND GetHWnd() const;
    RECT GetWindowClientRect() const;
    unsigned int GetWidth() const;
    unsigned int GetHeight() const;
    LPCWSTR GetTitle() const;
};


