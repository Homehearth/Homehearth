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
		std::vector<Element2D*> m_elements;
		draw_t m_startPos;
		sm::Vector2 m_currentPos;
		sm::Vector2 m_endPos;
		bool m_isPressed;

		void Update();

	public:

		/*
			Choose the start position of the scroller and the
			end position transitioned to when button is clicked.
		*/
		Scroller(const draw_t& startPos, const sm::Vector2& endPos);
		~Scroller();

		/*
			Add an element to the scroller. Any element added will be
			taken care of at the end of life.
		*/
		void AddElement(Element2D* elem);

		// Inherited via Element2D
		virtual void Draw() override;

		virtual void OnClick() override;

		virtual void OnHover() override;

		virtual bool CheckHover() override;

		virtual bool CheckClick() override;

	};
}