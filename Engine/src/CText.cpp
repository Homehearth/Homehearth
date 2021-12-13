#include "EnginePCH.h"
#include "CText.h"

rtd::CText::CText(const std::string& displayText, const draw_text_t& opts, const D2D1_COLOR_F& color)
{
    m_text = displayText;
    m_opts = opts;
    m_color = color;
}

void rtd::CText::SetPosition(const float& x, const float& y)
{
    m_opts.x_pos = x;
    m_opts.y_pos = y;
}

void rtd::CText::SetColor(const D2D1_COLOR_F& color)
{
    m_color = color;
}

void rtd::CText::SetText(const std::string& text)
{
    m_text = text;
}

void rtd::CText::Draw()
{
    D2D1Core::DrawT(m_text, m_color, m_opts);
}

void rtd::CText::OnClick()
{
}

void rtd::CText::OnHover()
{
}

bool rtd::CText::CheckHover()
{
    return false;
}

ElementState rtd::CText::CheckClick()
{
    return ElementState();
}
