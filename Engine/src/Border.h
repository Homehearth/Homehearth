#pragma once
#include "Element2D.h"

namespace rtd
{
	class Border : public Element2D
	{
	private:

		draw_shape_t m_borderOpts;
		draw_t m_opts;

	public:

		Border();
		Border(const draw_t& opts);
		Border(const draw_shape_t& shape, const draw_t& opts);

		// Update position of border.
		void UpdatePos(const draw_t& new_opts);

		// Inherited via Element2D
		virtual void Draw() override;

		// Inherited via Element2D
		virtual const bool IsClicked() override;

		// Inherited via Element2D
		virtual void OnClick() override;

	};
}