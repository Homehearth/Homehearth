#pragma once
#include "Element2D.h"
#include "Border.h"

namespace rtd
{
	/*
		Class used for rendering Blocks with D2D1
	*/
	class Canvas : public Element2D
	{
	private:

		D2D1_COLOR_F m_color;
		draw_t m_drawOpts;
		Border* m_border;

	public:

		/*
			Parameters: Color, Options, Name.
			Color is for prefered color of canvas.
			Options is for position and scale.
			Name is for identifying information.
		*/
		Canvas(const D2D1_COLOR_F& color, const draw_t& opts);
		~Canvas();

		Border* GetBorder();

		// Inherited via Element2D
		virtual void Draw() override;

		// Inherited via Element2D
		virtual const bool IsClicked() override;

		// Inherited via Element2D
		virtual void OnClick() override;

	};
}