#include "EnginePCH.h"
#include "InputSystem.h"
InputSystem::InputSystem(): m_kBState(), m_mouseState()
{
	m_mousePos.x = 0;
	m_mousePos.y = 0;
	m_keyboard = std::make_unique<dx::Keyboard>();
	m_kBTracker = std::make_unique<dx::Keyboard::KeyboardStateTracker>();
	m_mouse = std::make_unique<dx::Mouse>();
	m_mouseTracker = std::make_unique<dx::Mouse::ButtonStateTracker>();
	m_windowWidth = 0;
	m_windowHeight = 0;
	m_currentCamera = nullptr;
}

void InputSystem::SetMouseWindow(const HWND& windowHandle, const int width, const int height)
{
	m_mouse->SetWindow(windowHandle);
	//m_mouse->SetMode(DirectX::Mouse::MODE_RELATIVE);
	m_mouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);
	m_windowWidth = width;
	m_windowHeight = height;
}

void InputSystem::SetCamera(Camera* camera)
{
	if (camera != nullptr)
		this->m_currentCamera = camera;
	else
	{
		LOG_WARNING("Tried to set camera as nullptr...");
	}
}

void InputSystem::UpdateEvents()
{
	m_kBState = m_keyboard->GetState();
	m_kBTracker->Update(m_kBState);
	m_mouseState = m_mouse->GetState();
	m_mouseTracker->Update(m_mouseState);
	m_mousePos.x = m_mouseState.x;
	m_mousePos.y = m_mouseState.y;
}

bool InputSystem::CheckKeyboardKey(const dx::Keyboard::Keys& key, const KeyState state) const
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

bool InputSystem::CheckMouseKey(const MouseKey mouseButton, const KeyState state) const
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

int InputSystem::GetAxis(Axis axis) const
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

const MousePos& InputSystem::GetMousePos() const
{
	return m_mousePos;
}

void InputSystem::ToggleMouseVisibility() const
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

void InputSystem::UpdateMouseRay()
{
	if (m_currentCamera != nullptr)
	{
		// Transform from 2D view port coordinates to NDC -> also inverse projection to get to 4D view space
		const float viewX = (((2.0f * static_cast<float>(m_mousePos.x)) / (static_cast<float>(m_windowWidth))) - 1.0f) / m_currentCamera->GetProjection()._11;
		const float viewY = ((((2.0f * static_cast<float>(m_mousePos.y)) / static_cast<float>(m_windowHeight)) - 1.0f) * -1.0f) / m_currentCamera->GetProjection()._22;

		const sm::Matrix viewInverse = m_currentCamera->GetView().Invert();

		m_mouseRay.rayDir = sm::Vector3::TransformNormal({ viewX, viewY, 1.f }, viewInverse);
		m_mouseRay.rayPos = sm::Vector3::Transform({ 0.f,0.f,0.f }, viewInverse);
	}
	else
	{
		LOG_ERROR("Tried to update MouseRay when the camera is nullptr...");
	}

}

const Ray_t& InputSystem::GetMouseRay() const
{
	InputSystem::Get().UpdateMouseRay();
	return m_mouseRay;
}

const bool InputSystem::IsMouseRelative() const
{
	bool isRelative = false;
	if (m_mouseState.positionMode == dx::Mouse::MODE_RELATIVE)
	{
		isRelative = true;
	}
	return isRelative;
}

void InputSystem::SwitchMouseMode() const
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
