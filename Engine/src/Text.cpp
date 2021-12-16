#include "EnginePCH.h"
#include "Text.h"

using namespace rtd;

Text::Text(const std::string& displayText)
{
    m_text = displayText;
}

Text::Text(const std::string& displayText, const draw_text_t& opts)
{
    m_text = displayText;
    m_opts[0] = opts;
    m_opts[1] = opts;
}

void Text::SetText(const std::string& displayText)
{
    m_text = displayText;
}

const std::string& rtd::Text::GetText() const
{
    return m_text;
}

void rtd::Text::SetPosition(const float& x, const float& y)
{
    m_opts[0].x_pos = x;
    m_opts[0].y_pos = y;

    m_opts.Swap();
}

const sm::Vector2 rtd::Text::GetPosition()
{
    const sm::Vector2 pos = { m_opts[0].x_pos , m_opts[0].y_pos};
    return pos;
}

void rtd::Text::SetScale(float scale)
{
    m_opts[0].scale = scale;

    m_opts.Swap();
}

void rtd::Text::SetStretch(const float& x, const float& y)
{
    m_opts[0].x_stretch = x;
    m_opts[0].y_stretch = y;

    m_opts.Swap();
}

bool Text::SetFormat(const WCHAR* fontName, 
    IDWriteFontCollection* fontCollection, 
    const DWRITE_FONT_WEIGHT& weight, const DWRITE_FONT_STYLE& style, 
    const DWRITE_FONT_STRETCH& stretch, const FLOAT& fontSize,
    const WCHAR* localeName)
{
    return D2D1Core::CreateTextFormat(fontName, fontCollection, weight, style, stretch, fontSize, localeName, &m_opts[0].textFormat);
}

void Text::Draw()
{
    D2D1Core::DrawT(m_text, m_opts[1]);
    m_opts.ReadyForSwap();
}

void Text::OnClick()
{

}

void Text::OnHover()
{

}

bool Text::CheckHover()
{
    return false;
}

ElementState Text::CheckClick()
{
    return ElementState::NONE;
}
