#pragma once
#include "Element2D.h"

namespace rtd
{
	class Text : public Element2D
	{
	private:

		// Text to be rendered.
		std::string m_text;

		// Pointer to format as well as position, stretch area.
		draw_text_t m_opts;

	public:

		/*
			Using this construct will use the default text format defined in
			D2D1Core.h
		*/
		Text(const std::string& displayText);

		/*
			Use a custom font to display the text.
		*/
		Text(const std::string& displayText, const draw_text_t& opts);

		/*
			Set the text to display.
		*/
		void SetText(const std::string& displayText);

		/*
			Create a custom format to use for this text element.
		*/
		const bool SetFormat(const WCHAR* fontName,
			IDWriteFontCollection* fontCollection,
			const DWRITE_FONT_WEIGHT& weight, const DWRITE_FONT_STYLE& style,
			const DWRITE_FONT_STRETCH& stretch, const FLOAT& fontSize, const WCHAR* localeName);

		// Inherited via Element2D
		virtual void Draw() override;

		virtual void OnClick() override;

		virtual void OnHover() override;

		virtual const bool CheckHover() override;

		virtual const bool CheckClick() override;

	};
}