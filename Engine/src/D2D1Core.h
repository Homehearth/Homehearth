#pragma once
#include <EnginePCH.h>

class D2D1Core
{
private:
	IDWriteFactory* m_writeFactory;
	ID2D1Factory* m_factory;
	ID2D1RenderTarget* m_renderTarget;
	ID2D1HwndRenderTarget* m_hwndTarget;
	IDXGISurface1* m_surface;


	D2D1Core();
	~D2D1Core();
	const bool Setup(Window* window);
	static D2D1Core* instance;
public:

	// Boot up the D2D1Core application.
	static void Initialize(Window* window);

	// Free up any used memory.
	static void Destroy();

	static void DrawT(const std::string text, Window* window, IDWriteTextFormat* format = nullptr);

private:
	/*
		Default objects to be used when no other parameters are specified.
	*/

	IDWriteFont* m_writeFont;
	IDWriteTextFormat* m_writeFormat;
	ID2D1SolidColorBrush* m_solidBrush;
};
