#include "EnginePCH.h"
#include "MoneyUI.h"
#include <omp.h>
#include "Stats.h"
#include <cmath>

using namespace rtd;

void rtd::MoneyUI::Update()
{
    if ((omp_get_wtime() - lastUpdate) > ((double)Stats::Get().GetFrameTime() * 3.0))
    {
        if (m_localMoney < m_networkMoney)
            m_localMoney += 1;
        else
            m_localMoney -= 1;

        m_text->SetText("Money: " + std::to_string(m_localMoney));
        lastUpdate = omp_get_wtime();
    }
}

rtd::MoneyUI::MoneyUI(const draw_text_t& opts)
{
    m_text = std::make_unique<Text>("Money: 0", opts);
}

void rtd::MoneyUI::SetNewMoney(const uint32_t& moneyRef)
{
    m_networkMoney = moneyRef;
}

void MoneyUI::Draw()
{
    if (m_text)
        m_text->Draw();
}

void MoneyUI::OnClick()
{
    this->Update();
}

void MoneyUI::OnHover()
{
}

bool MoneyUI::CheckHover()
{
    return false;
}

bool MoneyUI::CheckClick()
{
    if (m_localMoney != m_networkMoney)
    {
        return true;
    }
    else
        return false;
}
