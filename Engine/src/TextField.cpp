#include "EnginePCH.h"
#include "TextField.h"

using namespace rtd;

rtd::TextField::TextField()
{
    m_text = std::make_unique<Text>("Text");
}

Text* rtd::TextField::GetText()
{
    return m_text.get();
}

Border* rtd::TextField::GetBorder()
{
    if (!m_border)
    {
        m_border = std::make_unique<Border>();
    }

    return m_border.get();
}

void rtd::TextField::Draw()
{
    if (m_text)
        m_text->Draw();
    if (m_border)
        m_border->Draw();
}

void rtd::TextField::OnClick()
{
}

void rtd::TextField::OnHover()
{
}

const bool rtd::TextField::CheckHover()
{
    return false;
}

const bool rtd::TextField::CheckClick()
{
    return false;
}
