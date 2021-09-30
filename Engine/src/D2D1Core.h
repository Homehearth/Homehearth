#pragma once
#include <EnginePCH.h>
#include "RTexture.h"

/*
	Is this your first time in this .h Header file?
	WIKI:

	The Draw options/functions are as follows:
	void DrawP() <-- Draw Picture {Draws a texture}
	void DrawF() <-- Draw Form {Draws one of the basic forms available}
	void DrawT() <-- Draw Text {Draws text at desired position}

	These draw options has different structure parameters.
	The structures can be seen below but for a quick summary:
	_DRAW(x, y, width, height) <-- Used when to draw textures and shapes.
	_DRAW_TEXT(format, x, y, x_stretch, y_stretch) <-- Used when to draw text.
	_DRAW_SHAPE(Shape, color) <-- Used for drawing shapes.

	Explanation:
	_DRAW(..., width, height), width and height represent the pixel
	width and height of the element.

	_DRAW_TEXT(format, ...), format is a IDWriteTextFormat pointer needed
	for drawing customized fonts or font scale and more. If however no
	format is input in DrawT() the default font will be used which is
	"Times New Roman" with size of 24.0f.

	_DRAW_SHAPE(Shape, color), Shape comes from the enum class Shapes
	which can be viewed below. These shapes are the currently available basic
	shapes.
	color is the desired color of the to be drawn shape. This requires
	D2D1::ColorF(float, float, float, float = 1.0f) to get input.

	Image Rendering:
	To start rendering an image you must first load it into the ResourceManager.
	The specified class has to be RBitMap since this is what we use to render 2d textures.
	Ex. auto* image = ResourceManager::GetResource<RBitMap>("oohstonefigures.jpg");
	When this is done use the D2D1Core to render your texture.
	Ex. D2D1Core::DrawP(_DRAW(), image);

	Troubleshooting:
	Make sure you have your Draw() function within the bounds of Being() and Present().
	Make sure your Draw isn't being obstructed by any other draws.
	If image is a nullptr nothing will get rendered.
*/

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
	float width = 250.0f;
	float height = 250.0f;

	// Layer for layered drawing.
	unsigned int layer = 0;

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
	NONE,
	RECTANGLE_FILLED,
	RECTANGLE_OUTLINED,
	TRIANGLE_FILLED,
	TRIANGLE_OUTLINED,
	NR_OF_SHAPES
};

/*
	Struct used to draw a shape with a specific color.
*/
struct _DRAW_SHAPE
{
	Shapes shape = Shapes::NONE;
	D2D1_COLOR_F color = D2D1::ColorF(1.0f, 1.0f, 1.0f);

	_DRAW_SHAPE()
	{
		// Empty
	}

	_DRAW_SHAPE(const Shapes& shape, float r, float g, float b, float a = 1.0f)
	{
		this->shape = shape;
		this->color = D2D1::ColorF(r, g, b, a);
	}

	_DRAW_SHAPE(const Shapes& shape, const D2D1_COLOR_F& color)
	{
		this->shape = shape;
		this->color = color;
	}
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
	static void DrawT(const std::string text = "Basic Text", const _DRAW_TEXT& opt = _DRAW_TEXT());

	/*
		Draws the specified shape with _DRAW input specifications.
		This method uses the default brush.
	*/
	static void DrawF(const _DRAW& fig, const _DRAW_SHAPE& shape);

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

	IDWriteTextFormat* m_writeFormat;
	ID2D1SolidColorBrush* m_solidBrush;
};
