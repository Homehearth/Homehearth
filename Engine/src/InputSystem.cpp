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

void InputSystem::RegisterEvent(const UINT& uMsg, const WPARAM& wParam)
{
	this->m_eventQueue.push({ uMsg, wParam });
}
bool InputSystem::PollEvent(InputEvent& event)
{
	if (!m_eventQueue.empty()) {
		event = m_eventQueue.front();
		m_eventQueue.pop();
		return true;
	}
	return false;
}

const std::unique_ptr<DirectX::Keyboard>& InputSystem::GetKeyboard() const
{
	return this->m_keyboard;
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
