#include "EnginePCH.h"
#include "MenuUI.h"

rtd::MenuUI::MenuUI(const std::string& texturePath, const draw_t& opts)
{
    m_texture = std::make_unique<Picture>(texturePath, opts);
	m_drawOpts = opts;
}

void rtd::MenuUI::SetOnPressedEvent(unsigned int index, std::function<void()> func)
{
	m_functions[index] = func;
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
	// Quit button
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos + m_drawOpts.width * 0.15f &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width * 0.85f &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos + m_drawOpts.height * 0.07f &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height * 0.30f)
	{
		//LOG_INFO("Hovering over Quit Button.")
		m_buttonHovering[0] = true;
	}

	// Continue button.
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos + m_drawOpts.width * 0.15f &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width * 0.85f &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos + m_drawOpts.height * 0.63f &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height * 0.91f)
	{
		//LOG_INFO("Hovering over Continue Button.")
		m_buttonHovering[2] = true;
	}

	// Background hover check.
	if (InputSystem::Get().GetMousePos().x > m_drawOpts.x_pos &&
		InputSystem::Get().GetMousePos().x < m_drawOpts.x_pos + m_drawOpts.width &&
		InputSystem::Get().GetMousePos().y > m_drawOpts.y_pos &&
		InputSystem::Get().GetMousePos().y < m_drawOpts.y_pos + m_drawOpts.height)
	{
		m_texture->SetOpacity(1.0f);
		return true;
	}

	m_buttonHovering[0] = false;
	m_buttonHovering[1] = false;
	m_buttonHovering[2] = false;

	m_texture->SetOpacity(.55f);
	return false;
}

ElementState rtd::MenuUI::CheckClick()
{
    if (CheckHover())
    {
		// CheckCollisions if mouse key is pressed.
		if (InputSystem::Get().CheckMouseKey(MouseKey::LEFT, KeyState::PRESSED))
		{
			if (m_buttonHovering[0])
			{
				if (m_functions[0])
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
					m_functions[2]();
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
