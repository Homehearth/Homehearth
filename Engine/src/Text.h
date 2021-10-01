#pragma once
#include "Element2D.h"

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

	// Inherited via Element2D
	virtual void Draw() override;

	virtual void OnClick() override;

	virtual void OnHover() override;

	virtual const bool CheckHover() override;

	virtual const bool CheckClick() override;

};