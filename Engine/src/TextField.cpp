#include "EnginePCH.h"
#include "TextField.h"
#include <conio.h>
#include <stdio.h>
using namespace rtd;


void rtd::TextField::UpdateInput()
{
    // Checks the 0-9 to A-Z
    for (int i = 0x30; i <= 0x5A; i++)
    {
        if ((GetAsyncKeyState(i) & WM_KEYUP) > 0)
        {
            const char c = static_cast<char>(i);
            m_stringText.push_back(c);   
        }
    }

    // Checks '.' press
    if ((GetAsyncKeyState(0xBE) & WM_KEYUP) > 0)
    {
        const char c = static_cast<char>(0x2E);
        m_stringText.push_back(c);   
    }

    // Remove with the backspace
    if (InputSystem::Get().CheckKeyboardKey(dx::Keyboard::Keys::Back, KeyState::PRESSED) && m_stringText.length() > 0)
    {
        m_stringText.pop_back();
    }

    if (m_timerForInputMarker.GetElapsedTime<std::chrono::milliseconds>() > 300)
    {
        m_isMarkerVisible = !m_isMarkerVisible;
        m_timerForInputMarker.Start();
    }
    
    if (m_isMarkerVisible)
    {
        m_stringText.push_back('|');
    }
    // Update the text
    m_text.get()->SetText(m_stringText);

    if (m_isMarkerVisible)
    {
        m_stringText.pop_back();
    }

}

rtd::TextField::TextField(const draw_text_t& opts)
{
    m_text = std::make_unique<Text>("", opts);
    m_opts = opts;
    m_isUsed = false;
    m_stringText = "";
    m_infoText = std::make_unique<Text>("Explanation Text", draw_text_t(opts.x_pos, opts.y_pos - 50.0f, opts.x_stretch, opts.y_stretch));

    m_isMarkerVisible = false;
}

Text* rtd::TextField::GetText()
{
    return m_infoText.get();
}

Border* rtd::TextField::GetBorder()
{
    if (!m_border)
    {
        m_border = std::make_unique<Border>(draw_t(m_opts.x_pos, m_opts.y_pos, m_opts.x_stretch, m_opts.y_stretch));
    }

    return m_border.get();
}

void rtd::TextField::SetIsUsed(bool used)
{
    m_isUsed = used;
    m_text.get()->SetText(m_stringText);
}

bool rtd::TextField::GetIsUsed() const
{
    return m_isUsed;
}

const bool rtd::TextField::GetBuffer(std::string*& output)
{    
    output = &m_stringText;
    return true;
}

void rtd::TextField::Draw()
{
    if (m_border)
        m_border->Draw();
    if (m_text)
        m_text->Draw();
    if (m_infoText)
        m_infoText->Draw();
}

void rtd::TextField::OnClick()
{
    SetIsUsed(!GetIsUsed());
    if (GetIsUsed()) 
    {
        m_timerForInputMarker.Start();
    }
    else
    {
        m_text.get()->SetText(m_stringText);
    }
}

void rtd::TextField::OnHover()
{
   
}

const bool rtd::TextField::CheckHover()
{
    // Is within bounds?
    if (InputSystem::Get().GetMousePos().x > m_opts.x_pos &&
        InputSystem::Get().GetMousePos().x < m_opts.x_pos + m_opts.x_stretch &&
        InputSystem::Get().GetMousePos().y > m_opts.y_pos &&
        InputSystem::Get().GetMousePos().y < m_opts.y_pos + m_opts.y_stretch)
    {
        m_isHovering = true;
        return true;
    }
    m_isHovering = false;
    return false;
}

const bool rtd::TextField::Update()
{

    m_isClicked = false;
    // Is within bounds?
    if (CheckHover())
    {
        OnHover();
    }
    if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED))
    {
        if (m_isHovering)
        {
            OnClick();
            m_isClicked = true;
        }
        else
        {
            SetIsUsed(false);
        }
    }
    if (GetIsUsed())
        this->UpdateInput();

    return true;
}
