#pragma once
#include <EnginePCH.h>

enum class Shapes
{
	DEFAULT,
	RECTANGLE_FILLED,
	RECTANGLE_OUTLINED,
	TRIANGLE_FILLED,
	TRIANGLE_OUTLINED,
	NR_OF_SHAPES
};

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

	/*
		Draw Text onto the window assigned in the parameter.
		This is a basic version of drawing text onto screen.
		Text will be drawn at the center of the window.
	*/
	static void DrawT(const std::string text, Window* window, IDWriteTextFormat* format = nullptr);

	/*
		Draws the specified shape with position upper_x, upper_y and width, height.
		Uses the default brush/color.
		Ex:
		upper_x = 0, upper_y = 0. width = window.width, height = window.height, shape = QUAD_FILLED
		will draw a quad over the entire screen.
	*/
	static void DrawF(const float upper_x, const float upper_y, const float width, const float height, const Shapes shape = Shapes::DEFAULT);

	/*
		Start buffering D2D1 Draw commands.
	*/
	static void Begin();

	/*
		End the buffering of D2D1 draw commands and present the image.
	*/
	static void Present();

private:
	/*
		Default objects to be used when no other parameters are specified.
	*/

	IDWriteFont* m_writeFont;
	IDWriteTextFormat* m_writeFormat;
	ID2D1SolidColorBrush* m_solidBrush;
};
