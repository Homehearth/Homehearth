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
		Shapes m_shape = Shapes::RECTANGLE_FILLED;

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
		void SetScale(const float& x_stretch, const float& y_stretch);

		// Set new color for canvas.
		void SetColor(const D2D1_COLOR_F& new_color);
		D2D1_COLOR_F& GetColor();
		const draw_t GetOpts() const;
		void SetBorderThickness(const LineWidth& thicc);
		void SetBorderShape(const Shapes& shape);
		void SetShape(const Shapes& shape);

		void SetBorderColor(const D2D1_COLOR_F& new_color);
		void HideBorder();
		void ShowBorder();
		void SetBorderWidth(const LineWidth& width);

		// Inherited via Element2D
		virtual void Draw() override;
		virtual void OnClick() override;
		virtual void OnHover() override;
		virtual ElementState CheckClick() override;

		// Inherited via Element2D
		virtual bool CheckHover() override;
	};
}