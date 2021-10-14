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
		std::unique_ptr<Border> m_border;

	public:

		TextField();

		Text* GetText();
		Border* GetBorder();


		// Inherited via Element2D
		virtual void Draw() override;

		virtual void OnClick() override;

		virtual void OnHover() override;

		virtual const bool CheckHover() override;

		virtual const bool CheckClick() override;

	};
}