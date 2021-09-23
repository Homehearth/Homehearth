#include "EnginePCH.h"
#include "InputSystem.h"
InputSystem::InputSystem()
{
	this->m_keyboard = std::make_unique<DirectX::Keyboard>();
	this->m_kBTracker = std::make_unique<DirectX::Keyboard::KeyboardStateTracker>();
	this->m_mouse = std::make_unique<DirectX::Mouse>();
	this->m_mouseTracker = std::make_unique<DirectX::Mouse::ButtonStateTracker>();
}

void InputSystem::SetMouseWindow(const HWND& windowHandle)
{
	this->m_mouse->SetWindow(windowHandle);
	this->m_mouse->SetMode(DirectX::Mouse::MODE_RELATIVE);
}

void InputSystem::UpdateEvents()
{
	this->m_kBState = this->m_keyboard->GetState();
	this->m_kBTracker->Update(this->m_kBState);
	this->m_mouseState = this->m_mouse->GetState();
	this->m_mouseTracker->Update(this->m_mouseState);
}

const bool InputSystem::CheckKeyboardKey(const DirectX::Keyboard::Keys& key, const KeyState state)
{
	switch (state)
	{
	case KeyState::PRESSED:
		return this->m_kBTracker->IsKeyPressed(key);
		break;
	case KeyState::RELEASED:
		return this->m_kBTracker->IsKeyReleased(key);
		break;
	case KeyState::HELD:
		return this->m_kBState.IsKeyDown(key);
		break;
	default:
		return false;
		break;
	}
}

const std::unique_ptr<DirectX::Keyboard>& InputSystem::GetKeyboard() const
{
	return this->m_keyboard;
}

const std::unique_ptr<DirectX::Mouse>& InputSystem::GetMouse() const
{
	return this->m_mouse;
}

const bool InputSystem::CheckMouseKey(const MouseKey mouseButton, const KeyState state)
{
	DirectX::Mouse::ButtonStateTracker::ButtonState* button = nullptr;
	switch (mouseButton)
	{
	case MouseKey::LEFT:
		button = &this->m_mouseTracker->leftButton;
		break;
	case MouseKey::MIDDLE:
		button = &this->m_mouseTracker->middleButton;
		break;
	case MouseKey::RIGHT:
		button = &this->m_mouseTracker->rightButton;
		break;
	default:
		button = &this->m_mouseTracker->leftButton;
		break;
	}
	switch (state)
	{
	case KeyState::PRESSED:
		return *button == this->m_mouseTracker->PRESSED;
		break;
	case KeyState::HELD:
		return *button == this->m_mouseTracker->HELD;
		break;
	case KeyState::RELEASED:
		return *button == this->m_mouseTracker->RELEASED;
		break;
	default:
		return false;
		break;
	}
}

const MousePos& InputSystem::GetMousePos() const
{
	MousePos position(this->m_mouseState.x, this->m_mouseState.y);
	return position;
}

void InputSystem::ToggleMouseVisibility()
{
	if (this->m_mouseState.positionMode == DirectX::Mouse::MODE_ABSOLUTE)
	{
		if (this->m_mouse->IsVisible())
		{
			this->m_mouse->SetVisible(false);
		}
		else
		{
			this->m_mouse->SetVisible(true);
		}
	}

}

void InputSystem::SwitchMouseMode()
{
	if (this->m_mouseState.positionMode == DirectX::Mouse::MODE_RELATIVE)
	{
		this->m_mouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);
	}
	else
	{
		this->m_mouse->SetMode(DirectX::Mouse::MODE_RELATIVE);
	}
}
