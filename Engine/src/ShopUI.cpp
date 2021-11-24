#include "EnginePCH.h"
#include "ShopUI.h"

using namespace rtd;

rtd::ShopUI::ShopUI(const std::string& filePath, const draw_t& opts)
{
    m_texture = std::make_unique<Picture>(filePath, opts);
    m_drawOpts = opts;
}

void rtd::ShopUI::SetOnPressedEvent(unsigned int index, std::function<void()> func)
{
	m_functions[index] = func;
}

void ShopUI::Draw()
{
    if (m_texture)
        m_texture->Draw();
}

void ShopUI::OnClick()
{
}

void ShopUI::OnHover()
{
}

bool ShopUI::CheckHover()
{
	// 1x1 defence button.
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos + m_drawOpts.width * 0.15f &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width * 0.45f &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos + m_drawOpts.height * 0.15f &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height * 0.45f)
	{
		m_buttonHovering[0] = true;
		//LOG_INFO("1x1 Defence hovering");
	}

	// 1x3 defence button.
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos + m_drawOpts.width * 0.52f &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width * 0.85f &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos + m_drawOpts.height * 0.15f &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height * 0.45f)
	{
		m_buttonHovering[1] = true;
		//LOG_INFO("1x3 Defence hovering");
	}

	// Attack button.
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos + m_drawOpts.width * 0.15f &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width * 0.33f &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos + m_drawOpts.height * 0.55f &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height * 0.75f)
	{
		m_buttonHovering[2] = true;
		//LOG_INFO("Attack Upgrade hovering");
	}

	// Armour button.
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos + m_drawOpts.width * 0.4f &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width * 0.6f &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos + m_drawOpts.height * 0.55f &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height * 0.75f)
	{
		m_buttonHovering[3] = true;
		//LOG_INFO("Armor Upgrade hovering");
	}

	// Health button.
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos + m_drawOpts.width * 0.61f &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width * 0.81f &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos + m_drawOpts.height * 0.55f &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height * 0.75f)
	{
		m_buttonHovering[4] = true;
		//LOG_INFO("Health Upgrade hovering");
	}

	// Main shop UI bounds.
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height)
	{
		m_texture->SetOpacity(1.0f);
		return true;
	}

	//LOG_INFO("NONE HOVERING");

	m_buttonHovering[0] = false;
	m_buttonHovering[1] = false;
	m_buttonHovering[2] = false;
	m_buttonHovering[3] = false;
	m_buttonHovering[4] = false;

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
					m_functions[0];
			}
			if (m_buttonHovering[1])
			{
				if (m_functions[1])
					m_functions[1];
			}
			if (m_buttonHovering[2])
			{
				if (m_functions[2])
					m_functions[2];
			}
			if (m_buttonHovering[3])
			{
				if (m_functions[3])
					m_functions[3];
			}
			if (m_buttonHovering[4])
			{
				if (m_functions[4])
					m_functions[4];
			}
			return ElementState::INSIDE;
		}
	}
	else
	{
		// CheckCollisions if mouse key is pressed.
		if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED))
		{
			return ElementState::OUTSIDE;
		}
	}

	return ElementState::NONE;
}
