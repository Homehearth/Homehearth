#include "EnginePCH.h"
#include "Scroller.h"

using namespace rtd;

void rtd::Scroller::Update()
{
    // Transition to the endPos position.
    sm::Vector2 temp = m_currentPos;
    if (m_isPressed)
    {
        m_currentPos = sm::Vector2::Lerp(m_currentPos, m_endPos, Stats::GetDeltaTime());
        m_canvas.get()->SetPosition(m_currentPos.x, m_currentPos.y);

        for (size_t i = 0; i < m_buttons.size(); i++)
        {
            m_buttons[i]->AddPosition(m_currentPos.x - temp.x, m_currentPos.y - temp.y);
            LOG_INFO("Pos x: %f, y: %f", m_buttons[i]->GetOpts().x_pos, m_buttons[i]->GetOpts().y_pos);

            // Mini update
            if (m_buttons[i]->CheckClick())
                m_buttons[i]->OnClick();
            if (m_buttons[i]->CheckHover())
                m_buttons[i]->OnHover();
        }
    }
    else
    {
        m_currentPos = sm::Vector2::Lerp(m_currentPos, sm::Vector2(m_startPos.x_pos, m_startPos.y_pos), Stats::GetDeltaTime());
        m_canvas.get()->SetPosition(m_currentPos.x, m_currentPos.y);

        for (size_t i = 0; i < m_buttons.size(); i++)
        {
            m_buttons[i]->AddPosition(m_currentPos.x - temp.x, m_currentPos.y - temp.y);
            LOG_INFO("Pos x: %f, y: %f", m_buttons[i]->GetOpts().x_pos, m_buttons[i]->GetOpts().y_pos);

            // Mini update
            if (m_buttons[i]->CheckClick())
                m_buttons[i]->OnClick();
            if (m_buttons[i]->CheckHover())
                m_buttons[i]->OnHover();
        }
    }
}

rtd::Scroller::Scroller(const draw_t& startPos, const sm::Vector2& endPos)
{
    m_startPos = startPos;
    m_endPos = endPos;
    m_isPressed = false;
    m_currentPos = { startPos.x_pos, startPos.y_pos };

    m_button = std::make_unique<Button>("pilNed.png", draw_t(0.0f, 0.0f, 32.0f, 32.0f));
    m_canvas = std::make_unique<Canvas>(startPos);
}

rtd::Scroller::~Scroller()
{
    while (!m_buttons.empty())
    {
       delete m_buttons[m_buttons.size() - 1];
       m_buttons[m_buttons.size() - 1] = nullptr;
       m_buttons.pop_back();
    }
}

void Scroller::Draw()
{
    if (m_canvas)
        m_canvas->Draw();
    if (m_button)
        m_button->Draw();
    for (size_t i = 0; i < m_buttons.size(); i++)
    {
        if (m_buttons[i])
            m_buttons[i]->Draw();
    }
}

void Scroller::OnClick()
{
    m_isPressed = !m_isPressed;
}

void Scroller::OnHover()
{
    this->Update();
}

/*
    Temp update.
*/
bool Scroller::CheckHover()
{
    return true;
}

bool Scroller::CheckClick()
{
    return m_button->CheckClick();
}
