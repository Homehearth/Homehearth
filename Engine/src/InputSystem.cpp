#include "EnginePCH.h"
#include "InputSystem.h"
InputSystem::InputSystem()
{
	m_keyboard = std::make_unique<dx::Keyboard>();
	m_kBTracker = std::make_unique<dx::Keyboard::KeyboardStateTracker>();
	m_mouse = std::make_unique<dx::Mouse>();
	m_mouseTracker = std::make_unique<dx::Mouse::ButtonStateTracker>();
}

void InputSystem::SetMouseWindow(const HWND& windowHandle)
{
	m_mouse->SetWindow(windowHandle);
	m_mouse->SetMode(DirectX::Mouse::MODE_RELATIVE);
}

void InputSystem::UpdateEvents()
{
	m_kBState = m_keyboard->GetState();
	m_kBTracker->Update(m_kBState);
	m_mouseState = m_mouse->GetState();
	m_mouseTracker->Update(m_mouseState);
}

const bool InputSystem::CheckKeyboardKey(const dx::Keyboard::Keys& key, const KeyState state) const
{
	switch (state)
	{
	case KeyState::PRESSED:
		return m_kBTracker->IsKeyPressed(key);
		break;
	case KeyState::RELEASED:
		return m_kBTracker->IsKeyReleased(key);
		break;
	case KeyState::HELD:
		return m_kBState.IsKeyDown(key);
		break;
	default:
		return false;
		break;
	}
}

const std::unique_ptr<dx::Keyboard>& InputSystem::GetKeyboard() const
{
	return m_keyboard;
}

const std::unique_ptr<dx::Mouse>& InputSystem::GetMouse() const
{
	return m_mouse;
}

const bool InputSystem::CheckMouseKey(const MouseKey mouseButton, const KeyState state) const
{
	dx::Mouse::ButtonStateTracker::ButtonState* button = nullptr;
	switch (mouseButton)
	{
	case MouseKey::LEFT:
		button = &m_mouseTracker->leftButton;
		break;
	case MouseKey::MIDDLE:
		button = &m_mouseTracker->middleButton;
		break;
	case MouseKey::RIGHT:
		button = &m_mouseTracker->rightButton;
		break;
	default:
		button = &m_mouseTracker->leftButton;
		break;
	}
	switch (state)
	{
	case KeyState::PRESSED:
		return *button == m_mouseTracker->PRESSED;
		break;
	case KeyState::HELD:
		return *button == m_mouseTracker->HELD;
		break;
	case KeyState::RELEASED:
		return *button == m_mouseTracker->RELEASED;
		break;
	default:
		return false;
		break;
	}
}

const int InputSystem::GetAxis(Axis axis) const
{
	int toReturn = 0;
	switch (axis)
	{
	case Axis::VERTICAL:
		if (CheckKeyboardKey(dx::Keyboard::W, KeyState::HELD) || CheckKeyboardKey(dx::Keyboard::Up, KeyState::HELD))
		{
			toReturn = 1;
		}
		else if (CheckKeyboardKey(dx::Keyboard::S, KeyState::HELD) || CheckKeyboardKey(dx::Keyboard::Down, KeyState::HELD))
		{
			toReturn = -1;
		}
		break;
	case Axis::HORIZONTAL:
		if (CheckKeyboardKey(dx::Keyboard::D, KeyState::HELD) || CheckKeyboardKey(dx::Keyboard::Right, KeyState::HELD))
		{
			toReturn = 1;
		}
		else if (CheckKeyboardKey(dx::Keyboard::A, KeyState::HELD) || CheckKeyboardKey(dx::Keyboard::Left, KeyState::HELD))
		{
			toReturn = -1;
		}
		break;
	default:
		break;
	}
	return toReturn;
}

const MousePos InputSystem::GetMousePos() const
{
	return { m_mouseState.x, m_mouseState.y };
}

void InputSystem::ToggleMouseVisibility()
{
	if (m_mouseState.positionMode == dx::Mouse::MODE_ABSOLUTE)
	{
		if (m_mouse->IsVisible())
		{
			m_mouse->SetVisible(false);
		}
		else
		{
			m_mouse->SetVisible(true);
		}
	}

}

void InputSystem::SwitchMouseMode()
{
	if (m_mouseState.positionMode == dx::Mouse::MODE_RELATIVE)
	{
		m_mouse->SetMode(dx::Mouse::MODE_ABSOLUTE);
	}
	else
	{
		m_mouse->SetMode(dx::Mouse::MODE_RELATIVE);
	}
}
