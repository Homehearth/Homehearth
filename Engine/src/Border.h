#pragma once
#include "Element2D.h"

namespace rtd
{
	class Border : public Element2D
	{
	private:

		draw_shape_t m_borderOpts;
		draw_t m_opts;
		LineWidth m_lineWidth = LineWidth::MEDIUM;
		ComPtr<ID2D1PathGeometry> m_geometry = nullptr;

	public:

		Border();
		Border(const draw_t& opts);
		Border(const draw_shape_t& shape, const draw_t& opts);

		// set the new color
		void SetColor(const D2D1_COLOR_F& new_color);
		D2D1_COLOR_F& GetColor();

		/*
			Change the shape into a circle. The radius is acting weirdly.
			To set it to perfectly round just set the radius to 1.0f
		*/
		bool SetRound(const float& radius);

		void SetLineWidth(const LineWidth& width);

		// Update position of border.
		void UpdatePos(const draw_t& new_opts);

		// Set the shape of the canvas.
		void SetShape(const draw_shape_t& new_shape);

		// Inherited via Element2D
		virtual void Draw() override;
		virtual void OnClick() override;
		virtual void OnHover() override;
		virtual bool CheckClick() override;


		// Inherited via Element2D
		virtual bool CheckHover() override;

	};
}