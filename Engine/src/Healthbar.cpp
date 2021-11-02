#include "EnginePCH.h"
#include "Healthbar.h"

using namespace rtd;

void rtd::Healthbar::Update()
{
    const float scale = *m_points / m_maxHealth;

}

rtd::Healthbar::Healthbar(void* health, const draw_t& drawOpts, const float& max_health)
{
    this->SetHealthVariable(health, max_health);
    m_drawOpts = drawOpts;
    m_backGround = std::make_unique<Canvas>(drawOpts);
    m_foreGround = std::make_unique<Canvas>(drawOpts);
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
        m_backGround->Draw();
    if (m_foreGround)
        m_foreGround->Draw();
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
