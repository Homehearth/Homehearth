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
		
		// Border color when actively used.
		D2D1_COLOR_F m_activeColor = {1.0f, 1.0f, 1.0f, 1.0f};

		// Border color when inactive.
		D2D1_COLOR_F m_inactiveColor = { 0.0f, 0.0f, 0.0f, 1.0f };
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

		// Set the active color and the inactive color.
		void SetBorderColors(const D2D1_COLOR_F& active, const D2D1_COLOR_F& inactive);

		// Reset the state of the textfield.
		void Reset();

		// Returns true if output is ready to be taken out.
		const bool GetBuffer(std::string*& output);

		// Get the buffer from textfield directly.
		std::string* RawGetBuffer();

		// Inherited via Element2D
		virtual void Draw() override;

		virtual void OnClick() override;

		virtual void OnHover() override;

		virtual const bool CheckHover() override;

		virtual const bool CheckClick() override;

	};
}