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
    m_opts = opts;
}

void Text::SetText(const std::string& displayText)
{
    m_text = displayText;
}

const bool Text::SetFormat(const WCHAR* fontName, 
    IDWriteFontCollection* fontCollection, 
    const DWRITE_FONT_WEIGHT& weight, const DWRITE_FONT_STYLE& style, 
    const DWRITE_FONT_STRETCH& stretch, const FLOAT& fontSize,
    const WCHAR* localeName)
{
    return D2D1Core::CreateTextFormat(fontName, fontCollection, weight, style, stretch, fontSize, localeName, &m_opts.textFormat);
}

void Text::Draw()
{
    D2D1Core::DrawT(m_text, m_opts);
}

void Text::OnClick()
{

}

void Text::OnHover()
{

}

const bool Text::CheckHover()
{
    return false;
}

const bool Text::CheckClick()
{
    return false;
}