#include "EnginePCH.h"
#include "Scroller.h"

using namespace rtd;

void rtd::Scroller::Update()
{
    // Transition to the endPos position.
    if (m_isPressed)
    {
        m_currentPos = sm::Vector2::Lerp(m_currentPos, m_endPos, Stats::Get().GetUpdateTime());
        m_canvas.get()->SetPosition(m_currentPos.x, m_currentPos.y);
    }
    else
    {
        m_currentPos = sm::Vector2::Lerp(m_currentPos, sm::Vector2(m_startPos.x_pos, m_startPos.y_pos), Stats::Get().GetUpdateTime());
        m_canvas.get()->SetPosition(m_currentPos.x, m_currentPos.y);
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
    while (!m_elements.empty())
    {
       delete m_elements[m_elements.size() - 1];
       m_elements[m_elements.size() - 1] = nullptr;
       m_elements.pop_back();
    }
}

void rtd::Scroller::AddElement(Element2D* elem)
{
    m_elements.push_back(elem);
}

void Scroller::Draw()
{
    if (m_canvas)
        m_canvas->Draw();
    if (m_button)
        m_button->Draw();
    for (size_t i = 0; i < m_elements.size(); i++)
    {
        if (m_elements[i])
            m_elements[i]->Draw();
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
