#pragma once
#include "Element2D.h"
#include "Button.h"
#include "Canvas.h"

namespace rtd
{
	/*
		A class representing a scrolling element being
		transitioned to a new position when activated.
	*/
	class Scroller : public Element2D
	{
	private:

		std::unique_ptr<Button> m_button;
		std::unique_ptr<Canvas> m_canvas;
		std::vector<Button*> m_buttons;
		draw_t m_startPos;
		sm::Vector2 m_currentPos;
		sm::Vector2 m_endPos;
		bool m_isPressed;
		std::function<void()> m_function;

		void Update();

	public:

		/*
			Choose the start position of the scroller and the
			end position transitioned to when button is clicked.
		*/
		Scroller(const draw_t& startPos, const sm::Vector2& endPos);
		~Scroller();
		void SetPrimeButtonMeasurements(const draw_t& opts);

		void SetOnPrimeButtonPress(std::function<void()> func);
		void SetOnPrimeButtonHover(std::function<void()> func);

		void ScrollUp();

		/*
			Add an element to the scroller. Any element added will be
			taken care of at the end of life.
		*/
		template <typename ...Args>
		rtd::Button* AddButton(Args... args);

		// Inherited via Element2D
		virtual void Draw() override;

		virtual void OnClick() override;

		virtual void OnHover() override;

		virtual bool CheckHover() override;

		virtual ElementState CheckClick() override;
	};

	template<typename ...Args>
	inline rtd::Button* Scroller::AddButton(Args ...args)
	{
		rtd::Button* elem = new rtd::Button(args...);
		if (elem)
		{
			m_buttons.push_back(elem);
			return elem;
		}
		else
			return nullptr;
	}
}