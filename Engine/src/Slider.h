#pragma once
#include "Handler2D.h"
#include "Border.h"
#include "Canvas.h"
#include "Text.h"

namespace rtd
{
	class Slider : public Element2D
	{
	private:

		std::unique_ptr<Border> m_border;
		std::unique_ptr<Canvas> m_slider;
		std::unique_ptr<Text> m_valueText;

		std::string m_explanationString;
		std::string m_valueString;
		float* m_value;
		draw_t m_drawOpts;
		bool m_isHeld = false;
		bool m_isHorizontal = false;
		sm::Vector2 m_minPos;
		sm::Vector2 m_maxPos;
		float m_maxVal = 0.0f;
		float m_minVal = 0.0f;

		void UpdateSliderPos(const float& x, const float& y);

	public:

		/*
			Create a slider to manipulate a value.
			The min and max represent the percentage of value bounds
			it is allowed within.
			Ex. value = 5, max = 2, min = 0.
			This means value max is 10(5 * 2), and value min is 0(5 * 0).
		*/
		Slider(D2D1_COLOR_F color, const draw_t& draw_opts, float* value, float max = 1.0f, float min = 0.0f, bool horizontal = true);

		Border* GetBorder();

		// Set the value to manipulate with the slider.
		void SetValue(float* value);

		void SetMinPos(sm::Vector2 minPos);
		void SetMaxPos(sm::Vector2 maxPos);

		void SetExplanationText(const std::string& text);

		// Inherited via Element2D
		virtual void Draw() override;
		virtual void OnClick() override;
		virtual void OnHover() override;
		virtual bool CheckHover() override;
		virtual ElementState CheckClick() override;
	};
}