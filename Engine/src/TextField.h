#pragma once
#include "Text.h"

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

		std::string m_stringText;
		std::unique_ptr<Border> m_border;
		draw_text_t m_opts;
		bool m_isUsed;
		bool m_finalInput = false;

		// Update the text on text field
		void Update();

	public:

		TextField(const draw_text_t& opts);

		// Returns the informational text about the textfield.
		Text* GetText();

		// Returns the border surrounding the text field.
		Border* GetBorder();

		// Returns true if output is ready to be taken out.
		const bool GetBuffer(std::string*& output);

		// Inherited via Element2D
		virtual void Draw() override;

		virtual void OnClick() override;

		virtual void OnHover() override;

		virtual const bool CheckHover() override;

		virtual const bool CheckClick() override;

	};
}