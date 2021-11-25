#include "EnginePCH.h"
#include "MoneyUI.h"
#include <omp.h>
#include "Stats.h"
#include <cmath>

using namespace rtd;

void rtd::MoneyUI::Update()
{
	if (m_localMoney < m_networkMoney)
	{
		m_localMoney += 1;
		m_text->SetText("Money: " + std::to_string(m_localMoney));
	}
	else if (m_localMoney > m_networkMoney)
	{
		m_localMoney -= 1;
		m_text->SetText("Money: " + std::to_string(m_localMoney));
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

const uint32_t& rtd::MoneyUI::GetNetworkMoney() const
{
	return m_networkMoney;
}

const uint32_t& rtd::MoneyUI::GetLocalMoney() const
{
	return m_localMoney;
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

ElementState MoneyUI::CheckClick()
{
	if (m_localMoney != m_networkMoney)
	{
		return ElementState::INSIDE;
	}
	else
		return ElementState::NONE;
}
