#include "EnginePCH.h"
#include "ShopUI.h"

/*
	DIFFERENT COSTS FOR THE SHOP. PURELY VISUAL NOTHING GAMEPLAY CHANGING.
*/
constexpr unsigned int TOWER_1X1_COST = 100;
constexpr unsigned int TOWER_1X3_COST = 250;
constexpr unsigned int PRIMARY_ABILITITY_COST = 300;
constexpr unsigned int HEAL_COST = 150;
constexpr unsigned int ARMOR_COST = 75; // Actually heal cost.
constexpr unsigned int REMOVE_DEFENCE_COST = 0;

using namespace rtd;

rtd::ShopUI::ShopUI(const std::string& filePath, const draw_t& opts)
{
	const unsigned int width = D2D1Core::GetWindow()->GetWidth();
	const unsigned int height = D2D1Core::GetWindow()->GetHeight();
	float widthScale = height * (16.f / 9.f);
    m_texture = std::make_unique<Picture>(filePath, opts);
	m_signTexture = std::make_unique<Picture>("NotEnoughMoneySign.png", draw_t(0.0f, 0.0f, widthScale * 0.15f, height * 0.075f));
	m_signText = std::make_unique<Text>("Money", draw_t(0.0f, 0.0f, widthScale * 0.15f, height * 0.075f));
    m_drawOpts = opts;
	m_chosenBorder = std::make_unique<Border>();
	m_chosenBorder->SetColor(D2D1::ColorF(0.0f, 1.0f, 0.0f));
	m_chosenBorder->SetVisiblity(false);
	m_chosenBorder->SetShape(Shapes::RECTANGLE_ROUNDED_OUTLINED);
}

void rtd::ShopUI::SetMoneyRef(MoneyUI* money)
{
	m_moneyRef = money;
}

void rtd::ShopUI::SetOnPressedEvent(unsigned int index, std::function<void()> func)
{
	m_functions[index] = func;
}

void rtd::ShopUI::SetOnHoverEvent(std::function<void()> func)
{
	m_onHoverFunction = func;
}

void ShopUI::Draw()
{
    if (m_texture)
        m_texture->Draw();

	if (m_chosenBorder)
	{
		if (m_chosenBorder->IsVisible())
		{
			m_chosenBorder->Draw();
		}
	}

	if (m_signTexture && m_signText)
	{
		if (m_signTexture->IsVisible())
		{
			m_signTexture->Draw();
			m_signText->Draw();
		}
	}

}

void ShopUI::OnClick()
{
}

void ShopUI::OnHover()
{
}

bool ShopUI::CheckHover()
{
	const unsigned int width = D2D1Core::GetWindow()->GetWidth();
	const unsigned int height = D2D1Core::GetWindow()->GetHeight();

	m_buttonHovering[0] = false;
	m_buttonHovering[1] = false;
	m_buttonHovering[2] = false;
	m_buttonHovering[3] = false;
	m_buttonHovering[4] = false;
	m_buttonHovering[5] = false;

	ElementState hoveringState = ElementState::NONE;

	// 1x1 defence button.
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos + m_drawOpts.width * 0.15f &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width * 0.475f &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos + m_drawOpts.height * 0.14f &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height * 0.40f)
	{
		m_buttonHovering[0] = true;
		//LOG_INFO("1x1 Defence hovering");
		m_signTexture->SetVisiblity(true);
		m_signTexture->SetPosition((FLOAT)InputSystem::Get().GetMousePos().x, (FLOAT)InputSystem::Get().GetMousePos().y);
		m_signText->SetPosition((FLOAT)InputSystem::Get().GetMousePos().x + width * 0.019f, (FLOAT)InputSystem::Get().GetMousePos().y);
		m_signText->SetText("Cost: " + std::to_string(TOWER_1X1_COST));

		if (m_moneyRef->GetNetworkMoney() >= TOWER_1X1_COST)
		{
			m_signTexture->SetTexture("EnoughMoneySign.png");
		}
		else
			m_signTexture->SetTexture("NotEnoughMoneySign.png");

		m_chosenBorder->UpdatePos(draw_t(m_drawOpts.x_pos + m_drawOpts.width * 0.15f, m_drawOpts.y_pos + m_drawOpts.height * 0.14f, (m_drawOpts.x_pos + m_drawOpts.width * 0.475f) - (m_drawOpts.width * 0.15f + m_drawOpts.x_pos), (m_drawOpts.y_pos + m_drawOpts.height * 0.4f) - (m_drawOpts.height * 0.14f + m_drawOpts.y_pos)));
		m_chosenBorder->SetVisiblity(true);

		hoveringState = ElementState::INSIDE;
	}

	// 1x3 defence button.
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos + m_drawOpts.width * 0.53f &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width * 0.85f &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos + m_drawOpts.height * 0.14f &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height * 0.415f)
	{
		m_buttonHovering[1] = true;
		//LOG_INFO("1x3 Defence hovering");
		m_signTexture->SetVisiblity(true);
		m_signTexture->SetPosition((FLOAT)InputSystem::Get().GetMousePos().x, (FLOAT)InputSystem::Get().GetMousePos().y);
		m_signText->SetPosition((FLOAT)InputSystem::Get().GetMousePos().x + width * 0.019f, (FLOAT)InputSystem::Get().GetMousePos().y);
		m_signText->SetText("Cost: " + std::to_string(TOWER_1X3_COST));

		if (m_moneyRef->GetNetworkMoney() >= TOWER_1X3_COST)
		{
			m_signTexture->SetTexture("EnoughMoneySign.png");
		}
		else
			m_signTexture->SetTexture("NotEnoughMoneySign.png");

		m_chosenBorder->UpdatePos(draw_t(m_drawOpts.x_pos + m_drawOpts.width * 0.53f, m_drawOpts.y_pos + m_drawOpts.height * 0.14f, (m_drawOpts.width * 0.85f + m_drawOpts.x_pos) - (m_drawOpts.width * 0.53f + m_drawOpts.x_pos), (m_drawOpts.height * 0.415f + m_drawOpts.y_pos) - (m_drawOpts.height * 0.14f + m_drawOpts.y_pos)));
		m_chosenBorder->SetVisiblity(true);

		hoveringState = ElementState::INSIDE;
	}

	// Attack button.
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos + m_drawOpts.width * 0.17f &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width * 0.35f &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos + m_drawOpts.height * 0.57f &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height * 0.715f)
	{
		m_buttonHovering[2] = true;
		//LOG_INFO("Attack Upgrade hovering");
		m_signTexture->SetVisiblity(true);
		m_signTexture->SetPosition((FLOAT)InputSystem::Get().GetMousePos().x, (FLOAT)InputSystem::Get().GetMousePos().y);
		m_signText->SetPosition((FLOAT)InputSystem::Get().GetMousePos().x + width * 0.019f, (FLOAT)InputSystem::Get().GetMousePos().y);
		
		if (m_atkUpgradeCount >= 3)
			m_signText->SetText("MAX LEVEL");
		else
			m_signText->SetText("Cost: " + std::to_string(PRIMARY_ABILITITY_COST));

		if (m_moneyRef->GetNetworkMoney() >= PRIMARY_ABILITITY_COST && m_atkUpgradeCount < 3)
		{
			m_signTexture->SetTexture("EnoughMoneySign.png");
		}
		else
			m_signTexture->SetTexture("NotEnoughMoneySign.png");

		m_chosenBorder->UpdatePos(draw_t(m_drawOpts.x_pos + m_drawOpts.width * 0.17f, m_drawOpts.y_pos + m_drawOpts.height * 0.57f, (m_drawOpts.x_pos + m_drawOpts.width * 0.35f) - (m_drawOpts.x_pos + m_drawOpts.width * 0.17f), (m_drawOpts.y_pos + m_drawOpts.height * 0.715f) - (m_drawOpts.y_pos + m_drawOpts.height * 0.57f)));
		m_chosenBorder->SetVisiblity(true);

		hoveringState = ElementState::INSIDE;
	}

	// Heal button.
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos + m_drawOpts.width * 0.41f &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width * 0.6f &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos + m_drawOpts.height * 0.56f &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height * 0.71f)
	{
		m_buttonHovering[3] = true;
		//LOG_INFO("Armor Upgrade hovering");
		m_signTexture->SetVisiblity(true);
		m_signTexture->SetPosition((FLOAT)InputSystem::Get().GetMousePos().x, (FLOAT)InputSystem::Get().GetMousePos().y);
		m_signText->SetPosition((FLOAT)InputSystem::Get().GetMousePos().x + width * 0.019f, (FLOAT)InputSystem::Get().GetMousePos().y);
		
		if (m_armorUpgradeCount >= 3)
			m_signText->SetText("MAX LEVEL");
		else
			m_signText->SetText("Cost: " + std::to_string(ARMOR_COST));

		if (m_moneyRef->GetNetworkMoney() >= ARMOR_COST)
		{
			m_signTexture->SetTexture("EnoughMoneySign.png");
		}
		else
			m_signTexture->SetTexture("NotEnoughMoneySign.png");

		m_chosenBorder->UpdatePos(draw_t(m_drawOpts.x_pos + m_drawOpts.width * 0.41f, m_drawOpts.y_pos + m_drawOpts.height * 0.56f, (m_drawOpts.x_pos + m_drawOpts.width * 0.6f) - (m_drawOpts.x_pos + m_drawOpts.width * 0.41f), (m_drawOpts.y_pos + m_drawOpts.height * 0.71f) - (m_drawOpts.y_pos + m_drawOpts.height * 0.56f)));
		m_chosenBorder->SetVisiblity(true);

		hoveringState = ElementState::INSIDE;
	}

	// Health button.
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos + m_drawOpts.width * 0.65f &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width * 0.85f &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos + m_drawOpts.height * 0.56f &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height * 0.715f)
	{
		m_buttonHovering[4] = true;
		//LOG_INFO("Health Upgrade hovering");
		m_signTexture->SetVisiblity(true);
		m_signTexture->SetPosition((FLOAT)InputSystem::Get().GetMousePos().x, (FLOAT)InputSystem::Get().GetMousePos().y);
		m_signText->SetPosition((FLOAT)InputSystem::Get().GetMousePos().x + width * 0.019f, (FLOAT)InputSystem::Get().GetMousePos().y);
		if (m_healthUpgradeCount >= 3)
			m_signText->SetText("MAX LEVEL");
		else
			m_signText->SetText("Cost: " + std::to_string(HEAL_COST));

		if (m_moneyRef->GetNetworkMoney() >= HEAL_COST && m_healthUpgradeCount < 3)
		{
			m_signTexture->SetTexture("EnoughMoneySign.png");
		}
		else
			m_signTexture->SetTexture("NotEnoughMoneySign.png");

		m_chosenBorder->UpdatePos(draw_t(m_drawOpts.x_pos + m_drawOpts.width * 0.66f, m_drawOpts.y_pos + m_drawOpts.height * 0.56f, (m_drawOpts.x_pos + m_drawOpts.width * 0.85f) - (m_drawOpts.x_pos + m_drawOpts.width * 0.66f), (m_drawOpts.y_pos + m_drawOpts.height * 0.715f) - (m_drawOpts.y_pos + m_drawOpts.height * 0.56f)));
		m_chosenBorder->SetVisiblity(true);

		hoveringState = ElementState::INSIDE;
	}

	// Remove defence button.
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos + m_drawOpts.width * 0.18f &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width * 0.77f &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos + m_drawOpts.height * 0.80f &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height)
	{
		m_buttonHovering[5] = true;

		m_signTexture->SetVisiblity(true);
		m_signTexture->SetPosition((FLOAT)InputSystem::Get().GetMousePos().x, (FLOAT)InputSystem::Get().GetMousePos().y);
		m_signText->SetPosition((FLOAT)InputSystem::Get().GetMousePos().x + width * 0.019f, (FLOAT)InputSystem::Get().GetMousePos().y);
		m_signText->SetText("Cost: " + std::to_string(REMOVE_DEFENCE_COST));
		
		if (m_moneyRef->GetNetworkMoney() >= REMOVE_DEFENCE_COST)
		{
			m_signTexture->SetTexture("EnoughMoneySign.png");
		}
		else
			m_signTexture->SetTexture("NotEnoughMoneySign.png");

		m_chosenBorder->UpdatePos(draw_t(m_drawOpts.x_pos + m_drawOpts.width * 0.18f, m_drawOpts.y_pos + m_drawOpts.height * 0.80f, (m_drawOpts.x_pos + m_drawOpts.width * 0.77f) - (m_drawOpts.x_pos + m_drawOpts.width * 0.18f), (m_drawOpts.y_pos + m_drawOpts.height) - (m_drawOpts.y_pos + m_drawOpts.height * 0.80f)));
		m_chosenBorder->SetVisiblity(true);

		hoveringState = ElementState::INSIDE;
	}

	if (hoveringState == ElementState::NONE)
	{
		m_chosenBorder->SetVisiblity(false);
		m_signTexture->SetVisiblity(false);
	}

	// Main shop UI bounds.
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height)
	{
		m_texture->SetOpacity(1.0f);
		if (m_onHoverFunction)
		{
			m_onHoverFunction();
		}

		return true;
	}

	m_texture->SetOpacity(0.45f);
	return false;
}

ElementState ShopUI::CheckClick()
{
	if (CheckHover())
	{
		// CheckCollisions if mouse key is pressed.
		if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED))
		{
			if (m_buttonHovering[0])
			{
				if(m_functions[0])
					m_functions[0]();
			}
			if (m_buttonHovering[1])
			{
				if (m_functions[1])
					m_functions[1]();
			}
			if (m_buttonHovering[2])
			{
				if (m_functions[2])
				{
					m_functions[2]();
					if (m_moneyRef->GetNetworkMoney() > PRIMARY_ABILITITY_COST)
						m_atkUpgradeCount++;
				}
					
			}
			if (m_buttonHovering[3])
			{
				if (m_functions[3])
				{
					m_functions[3]();
				}
					
					
			}
			if (m_buttonHovering[4])
			{
				if (m_functions[4])
				{
					m_functions[4]();
					if (m_moneyRef->GetNetworkMoney() > HEAL_COST)
						m_healthUpgradeCount++;
				}
					
			}
			if (m_buttonHovering[5])
			{
				if (m_functions[5])
					m_functions[5]();
			}
			return ElementState::INSIDE;
		}
	}
	else
	{
		return ElementState::OUTSIDE;
	}

	return ElementState::NONE;
}
