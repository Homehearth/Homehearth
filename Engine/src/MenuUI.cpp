#include "EnginePCH.h"
#include "MenuUI.h"

rtd::MenuUI::MenuUI(const std::string& texturePath, const draw_t& opts)
{
    m_texture = std::make_unique<Picture>(texturePath, opts);
	m_drawOpts = opts;
}

void rtd::MenuUI::Draw()
{
    if (m_texture)
        m_texture->Draw();
}

void rtd::MenuUI::OnClick()
{

}

void rtd::MenuUI::OnHover()
{

}

bool rtd::MenuUI::CheckHover()
{
	// Background hover check.
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height)
	{
		m_texture->SetOpacity(1.0f);
		return true;
	}

	m_texture->SetOpacity(.75f);
	return false;
}

ElementState rtd::MenuUI::CheckClick()
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
