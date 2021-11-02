#include "EnginePCH.h"
#include "Healthbar.h"

using namespace rtd;

void rtd::Healthbar::Update()
{
    // Update if health is not nullptr
    if (m_points)
    {
        // Scale the foreground to reflect on available health.
        const float scale = *m_points / m_maxHealth;
        m_drawOpts.width = m_sizeFull * scale;

        m_foreGround.get()->SetPosition(m_drawOpts.x_pos, m_drawOpts.y_pos);
        m_foreGround.get()->SetScale(m_drawOpts.width, m_drawOpts.height);
    }
}

rtd::Healthbar::Healthbar(void* health, const draw_t& drawOpts, const float& max_health)
{
    this->SetHealthVariable(health, max_health);
    m_drawOpts = drawOpts;
    m_backGround = std::make_unique<Canvas>(m_drawOpts);
    m_foreGround = std::make_unique<Canvas>(m_drawOpts);
    m_sizeFull = drawOpts.width;

    m_backGround.get()->SetColor(D2D1::ColorF(134 / 255.0f, 2 / 255.0f, 17 / 255.0f));
    m_foreGround.get()->SetColor(D2D1::ColorF(61 / 255.0f, 121 / 255.0f, 15 / 255.0f));
}

rtd::Healthbar::~Healthbar()
{

}

void rtd::Healthbar::SetHealthVariable(void* var, const float& max_health)
{
    m_points = (float*)var;
    m_maxHealth = max_health;
}

void rtd::Healthbar::Draw()
{
    if (m_backGround)
        m_backGround.get()->Draw();
    if (m_foreGround)
        m_foreGround.get()->Draw();
}


/*
    Leave blank for now.
*/
void rtd::Healthbar::OnClick()
{
}

void rtd::Healthbar::OnHover()
{
    this->Update();
}

bool rtd::Healthbar::CheckHover()
{
    return true;
}

bool rtd::Healthbar::CheckClick()
{
    return false;
}
