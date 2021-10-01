#include "EnginePCH.h"
#include "Text.h"

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
