#include "EnginePCH.h"
#include "Slider.h"
#include <iomanip>

using namespace rtd;

void rtd::Slider::UpdateSliderPos(const float& x, const float& y)
{
	if(x >= m_minPos.x && x <= m_maxPos.x)
		m_drawOpts.x_pos = x;
	if(y >= m_minPos.y && y <= m_maxPos.y)
		m_drawOpts.y_pos = y;
}

rtd::Slider::Slider(D2D1_COLOR_F color, const draw_t& draw_opts, float* value, float max, float min, bool horizontal)
{
    m_slider = std::make_unique<Canvas>(color, draw_opts);
    m_value = value;
	m_drawOpts = draw_opts;
	m_isHorizontal = horizontal;
	m_maxVal = max * *value;
	m_minVal = min * *value;

	std::string tmp = std::to_string(*m_value);
	auto length = std::snprintf(&m_valueString[0], m_valueString.size(), "%.2f", *m_value);
	tmp.resize(length);
	m_valueText = std::make_unique<Text>(tmp, draw_text_t(draw_opts.x_pos + (draw_opts.width * 0.5f), draw_opts.y_pos - draw_opts.height, tmp.length() * 24.0f, draw_opts.height));

	if (m_isHorizontal)
	{
		m_minPos = { m_drawOpts.x_pos, m_drawOpts.y_pos };
		m_maxPos = { m_drawOpts.x_pos + 100.0f, m_drawOpts.y_pos };
	}
	else
	{
		m_minPos = { m_drawOpts.x_pos, m_drawOpts.y_pos };
		m_maxPos = { m_drawOpts.x_pos + 100.0f, m_drawOpts.y_pos };
	}
}

Border* rtd::Slider::GetBorder()
{
    if (!m_border)
    {
        m_border = std::make_unique<Border>(m_drawOpts);
    }
    return m_border.get();
}

void rtd::Slider::SetValue(float* value)
{
	m_value = value;
}

void rtd::Slider::SetMinPos(sm::Vector2 minPos)
{
	m_minPos = minPos;
}

void rtd::Slider::SetMaxPos(sm::Vector2 maxPos)
{
	m_maxPos = maxPos;
}

void Slider::Draw()
{
	if (m_border)
		m_border->Draw();
	if (m_slider)
		m_slider->Draw();
	if (m_valueText)
		m_valueText->Draw();
}

void Slider::OnClick()
{
	// Update the position of slider.
	if (m_isHorizontal)
		UpdateSliderPos(InputSystem::Get().GetMousePos().x - (m_drawOpts.width * 0.5f), m_drawOpts.y_pos);
	else
		UpdateSliderPos(m_drawOpts.x_pos, InputSystem::Get().GetMousePos().y - (m_drawOpts.height * 0.5f));

	m_slider.get()->SetPosition(m_drawOpts.x_pos, m_drawOpts.y_pos);
	// Update the value
	if (m_value)
	{
		m_valueString = std::to_string(*m_value);
		auto length = std::snprintf(&m_valueString[0], m_valueString.size(), "%.2f", *m_value);
		m_valueString.resize(length);
		m_valueText.get()->SetText(m_valueString);
		if(m_isHorizontal)
			*m_value = (m_maxPos.x - m_drawOpts.x_pos) * 0.01f * m_maxVal;
		else
			*m_value = (m_maxPos.y - m_drawOpts.y_pos) * 0.01f * m_maxVal;
	}
}

void Slider::OnHover()
{
}

const bool Slider::CheckHover()
{
	return false;
}

const bool Slider::CheckClick()
{
	m_isHeld = false;
	// CheckCollisions if mouse key is pressed.
	if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::HELD))
	{
		// Is within bounds?
		if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos &&
			InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width &&
			InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos &&
			InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height)
		{
			m_isHeld = true;
		}
	}

	return m_isHeld;
}
