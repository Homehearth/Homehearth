#pragma once
#include "Element2D.h"

namespace rtd
{
	class CText : public Element2D
	{
	private:

		draw_text_t m_opts;
		D2D1_COLOR_F m_color = { 0.0f, 0.0f, 0.0f, 1.0f };
		std::string m_text;

	public:

		CText(const std::string& displayText, const draw_text_t& opts, const D2D1_COLOR_F& color);

		void SetPosition(const float& x, const float& y);
		void SetColor(const D2D1_COLOR_F& color);
		void SetText(const std::string& text);

		// Inherited via Element2D
		virtual void Draw() override;

		virtual void OnClick() override;

		virtual void OnHover() override;

		virtual bool CheckHover() override;

		virtual ElementState CheckClick() override;

	};
}