#pragma once
#include <EnginePCH.h>
#include "RTexture.h"

/*
	Struct used to set position and scale 
	of drawable element.
*/
struct _DRAW
{
	/*
		The upper left corner position.
	*/
	float x_pos = 0.0f;
	float y_pos = 0.0f;

	/*
		Width and Height scale.
	*/
	float width = 1.0f;
	float height = 1.0f;

	_DRAW()
	{
		//Empty
	}

	_DRAW(float x_pos, float y_pos, float width, float height)
	{
		this->x_pos = x_pos;
		this->y_pos = y_pos;
		this->width = width;
		this->height = height;
	}
};

/*
	Struct to Define text format
	as well as position and stretch area.
*/
struct _DRAW_TEXT
{
	// Text format.
	IDWriteTextFormat* textFormat = nullptr;

	/*
		Upper Left Corner position.
	*/
	float x_pos = 1.0f;
	float y_pos = 1.0f;

	/*
		Lower Right Corner position.
	*/
	float x_stretch = 100.0f;
	float y_stretch = 100.0f;

	_DRAW_TEXT()
	{
		// Empty
	}

	_DRAW_TEXT(IDWriteTextFormat* format, float x, float y, float x_stretch, float y_stretch)
	{
		textFormat = format;
		this->x_pos = x;
		this->y_pos = y;
		this->x_stretch = x_stretch;
		this->y_stretch = y_stretch;
	}

	_DRAW_TEXT(float x, float y, float x_stretch, float y_stretch)
	{
		this->x_pos = x;
		this->y_pos = y;
		this->x_stretch = x_stretch;
		this->y_stretch = y_stretch;
	}
};

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
	IWICImagingFactory* m_imageFactory;
	Window* m_windowPointer;

private:
	D2D1Core();
	~D2D1Core();
	const bool Setup(Window* window);
	static D2D1Core* instance;

	/*
		Loads in a file and converts it into a ID2D1Bitmap.
	*/
	HRESULT LoadBitMap(
		const LPCWSTR& filePath,
		ID2D1Bitmap** bitMap);
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
	static void DrawT(const std::string text, const _DRAW_TEXT& opt = _DRAW_TEXT());

	/*
		Draws the specified shape with position upper_x, upper_y and width, height.
		Uses the default brush/color.
	*/
	static void DrawF(const _DRAW& fig, const Shapes shape = Shapes::DEFAULT);

	/*
		Draws a Bitmap onto the screen at the coordinates of
		_DRAW specification.
	*/
	static void DrawP(const _DRAW& fig, ID2D1Bitmap* texture = nullptr);

	/*
		Run buffering D2D1 Draw commands.
	*/
	static void Begin();

	/*
		End the buffering of D2D1 draw commands and present the image.
	*/
	static void Present();

	/*
		Load an image from file into memory.
		Second parameter is for taking out the pointer for further
		usage if needed.
	*/
	static const bool CreateImage(const std::string& filename, ID2D1Bitmap** p_pointer = nullptr);

private:
	/*
		Default objects to be used when no other parameters are specified.
	*/

	IDWriteFont* m_writeFont;
	IDWriteTextFormat* m_writeFormat;
	ID2D1SolidColorBrush* m_solidBrush;
};
