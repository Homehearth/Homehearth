#pragma once
#include "Element2D.h"
#include "Border.h"

namespace rtd
{
	class Button : public Element2D
	{
	private:

		draw_t m_drawOpts;
		D2D1_COLOR_F m_color = D2D1::ColorF(.0f, 1.0f, .5f);
		std::unique_ptr<Border> m_border = nullptr;
		std::unique_ptr<Picture> m_picture = nullptr;
		std::unique_ptr<Canvas> m_canvas = nullptr;

	public:

		Button();
		~Button();
		Button(const std::string& fileName, const draw_t& opts, const bool border = false);

		Border* GetBorder();
		Picture* GetPicture();
		Canvas* GetCanvas();

		// CheckCollisions if the button is clicked.
		const bool CheckClicked() const;

		// Inherited via Element2D
		virtual void Draw() override;
		virtual void OnClick() override;
		virtual void OnHover() override;
		virtual const bool CheckClick() override;
		virtual const bool CheckHover() override;

	};
}