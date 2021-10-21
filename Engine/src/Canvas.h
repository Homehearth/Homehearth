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
		std::unique_ptr<Border> m_border;

	public:

		/*
			Parameters: Color, Options, Name.
			Color is for prefered color of canvas.
			Options is for position and scale.
			Name is for identifying information.
		*/
		Canvas(const D2D1_COLOR_F& color, const draw_t& opts);
		Canvas(const draw_t& opts);
		Canvas();
		~Canvas();

		void SetPosition(const float& x, const float& y);

		// Set new color for canvas.
		void SetColor(const D2D1_COLOR_F& new_color);
		D2D1_COLOR_F& GetColor();

		Border* GetBorder();

		// Inherited via Element2D
		virtual void Draw() override;
		virtual void OnClick() override;
		virtual void OnHover() override;
		virtual const bool Update() override;

		// Inherited via Element2D
		virtual const bool CheckHover() override;
	};
}