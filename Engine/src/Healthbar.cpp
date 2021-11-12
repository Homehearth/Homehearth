#include "EnginePCH.h"
#include "Healthbar.h"
#include "Components.h"

constexpr float SHOW_TIME = 3.0f;

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
            m_drawOpts[0].width = (m_sizeFull * scale) > 0 ? (m_sizeFull * scale) : 0;

            //m_foreGround.get()->SetPosition(m_drawOpts[0].x_pos, m_drawOpts[0].y_pos);
            m_foreGround.get()->SetScale(m_drawOpts[0].width, m_drawOpts[0].height);
            //m_healthInfo.get()->SetText("Health: " + std::to_string((int)h->currentHealth));
        }
    }
}

rtd::Healthbar::Healthbar(const draw_t& drawOpts)
{
    m_drawOpts[0] = drawOpts;
    m_backGround = std::make_unique<Canvas>(m_drawOpts[0]);
    m_foreGround = std::make_unique<Canvas>(m_drawOpts[0]);
    //m_healthInfo = std::make_unique<Text>("Health", draw_text_t(m_drawOpts[0].x_pos, m_drawOpts[0].y_pos, m_drawOpts[0].width, m_drawOpts[0].height));
    //m_healthInfo.get()->SetVisiblity(false);
    m_sizeFull = drawOpts.width;

    m_drawOpts[1] = m_drawOpts[0];

    m_backGround.get()->SetColor(D2D1::ColorF(134 / 255.0f, 2 / 255.0f, 17 / 255.0f));
    m_foreGround.get()->SetColor(D2D1::ColorF(61 / 255.0f, 121 / 255.0f, 15 / 255.0f));
    m_backGround.get()->SetBorderColor(D2D1::ColorF(0.0f, 0.0f, 0.0f));
    m_backGround.get()->ShowBorder();
    m_backGround.get()->SetBorderWidth(LineWidth::SMALL);
}

rtd::Healthbar::~Healthbar()
{

}

void rtd::Healthbar::SetHealthVariable(Entity e)
{
    m_entity = e;
}

void rtd::Healthbar::SetPosition(const float& x, const float& y)
{
    m_drawOpts[0].x_pos = x;
    m_drawOpts[0].y_pos = y;
    m_drawOpts.Swap();

    m_drawOpts[0].height = m_drawOpts[1].height;
}

void rtd::Healthbar::SetStretch(const float& x, const float& y)
{
    m_drawOpts[0].height = y;
    m_drawOpts[0].width = x;
    m_foreGround.get()->SetScale(x, y);
    m_backGround.get()->SetScale(x, y);
    m_sizeFull = x;
}

const draw_t rtd::Healthbar::GetOpts() const
{
    return m_backGround.get()->GetOpts();
}

void rtd::Healthbar::Draw()
{
    {
        float x = m_drawOpts[1].x_pos;
        float y = m_drawOpts[1].y_pos;
        m_backGround.get()->SetPosition(x, y);
        m_foreGround.get()->SetPosition(x, y);
        //m_healthInfo.get()->SetPosition(x, y);
    }

    if (m_backGround)
        m_backGround.get()->Draw();
    if (m_foreGround)
        m_foreGround.get()->Draw();

    //if (m_healthInfo.get()->IsVisible())
    //    m_healthInfo.get()->Draw();

    m_drawOpts.ReadyForSwap();
}


/*
    Leave blank for now.
*/
void rtd::Healthbar::OnClick()
{
    this->Update();
}

void rtd::Healthbar::OnHover()
{
  /*  m_healthInfo.get()->SetVisiblity(true);*/
}

bool rtd::Healthbar::CheckHover()
{
    //// Is within bounds?
    //if (InputSystem::Get().GetMousePos().x > m_drawOpts[1].x_pos &&
    //    InputSystem::Get().GetMousePos().x < m_drawOpts[1].x_pos + m_drawOpts[1].width &&
    //    InputSystem::Get().GetMousePos().y > m_drawOpts[1].y_pos &&
    //    InputSystem::Get().GetMousePos().y < m_drawOpts[1].y_pos + m_drawOpts[1].height)
    //{
    //    return true;
    //}
    
    //m_healthInfo.get()->SetVisiblity(false);
    return false;
}

bool rtd::Healthbar::CheckClick()
{
    return true;
}
