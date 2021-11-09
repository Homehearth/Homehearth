#pragma once
#include "Element2D.h"
#include "Border.h"
#include "Text.h"
#include "Picture.h"
#include "Canvas.h"

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
		std::unique_ptr<Text> m_text = nullptr;

		std::function<void()> m_function = nullptr;

	public:

		Button();
		~Button();
		Button(const std::string& fileName, const draw_t& opts, const bool border = false);

		Border* GetBorder();
		Picture* GetPicture();
		Canvas* GetCanvas();
		Text* GetText();
		const draw_t& GetOpts() const;

		void SetPosition(const float& x, const float& y);
		void AddPosition(const float& x, const float& y);

		void SetOnPressedEvent(const std::function<void()>& func);

		// CheckCollisions if the button is clicked.
		bool CheckClicked() const;

		// Inherited via Element2D
		virtual void Draw() override;
		virtual void OnClick() override;
		virtual void OnHover() override;
		virtual bool CheckClick() override;
		virtual bool CheckHover() override;

	};
}