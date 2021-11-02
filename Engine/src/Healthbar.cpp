#include "EnginePCH.h"
#include "Healthbar.h"
#include "Components.h"

using namespace rtd;

void rtd::Healthbar::Update()
{
    // Update if health is not nullptr
    if (!m_entity.IsNull())
    {
        comp::Health* h = m_entity.GetComponent<comp::Health>();

        if (h)
        {
            // Scale the foreground to reflect on available health.
            const float scale = h->currentHealth / h->maxHealth;
            m_drawOpts.width = (m_sizeFull * scale) > 0 ? (m_sizeFull * scale) : 0;

            m_foreGround.get()->SetPosition(m_drawOpts.x_pos, m_drawOpts.y_pos);
            m_foreGround.get()->SetScale(m_drawOpts.width, m_drawOpts.height);
        }
    }
}

rtd::Healthbar::Healthbar(const draw_t& drawOpts)
{
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

void rtd::Healthbar::SetHealthVariable(Entity e)
{
    m_entity = e;
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
