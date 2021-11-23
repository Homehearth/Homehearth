#include "EnginePCH.h"
#include "ShopUI.h"

using namespace rtd;

rtd::ShopUI::ShopUI(const std::string& filePath, const draw_t& opts)
{
    m_texture = std::make_unique<Picture>(filePath, opts);
    m_drawOpts = opts;
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
	// Is within bounds?
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height)
	{
		m_texture->SetOpacity(1.0f);
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
