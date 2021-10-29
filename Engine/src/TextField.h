#pragma once
#include "Text.h"
#include "Border.h"
#include "Canvas.h"

namespace rtd
{
	/*
		A class representing a text field being clickable and taking input from user.
	*/
	class TextField : public Element2D
	{
	private:
		std::unique_ptr<Text> m_text;

		// Text used to explain the text fields purpose.
		std::unique_ptr<Text> m_infoText;
		std::unique_ptr<Canvas> m_canvas;

		std::string m_stringText;
		size_t m_textLimit;
		std::unique_ptr<Border> m_border;
		draw_text_t m_opts;
		
		// Border color when actively used.
		D2D1_COLOR_F m_activeColor;

		bool m_isUsed;
		bool m_finalInput;

		// Update the text on text field
		void Update();

	public:
		TextField(const draw_text_t& opts, size_t textLimit = -1, bool isUsed = false, D2D1_COLOR_F borderActive = { 0.0f, 0.0f, 0.0f, 1.0f });

		void SetDescriptionText(const std::string& displayText);

		// Get the buffer from textfield directly.
		std::string* RawGetBuffer();

		// Inherited via Element2D
		virtual void Draw() override;

		virtual void OnClick() override;

		virtual void OnHover() override;

		virtual bool CheckHover() override;

		virtual bool CheckClick() override;

	};
}