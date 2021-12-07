#include "EnginePCH.h"
#include "InputSystem.h"
InputSystem::InputSystem() : m_kBState(), m_mouseState()
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
	m_lastScrollValue = 0;
	m_lastScrollDirection = 0;
	m_inputState = SystemState::TEXTFIELD;
	m_ctrlMode = false;
	m_shiftMode = false;
	if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0)
	{
		m_capsLockMode = true;
	}
	else
	{
		m_capsLockMode = false;
	}
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
	{
		this->m_currentCamera = camera;
	}
	else
	{
		LOG_WARNING("Tried to set camera with nullptr...");
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
void InputSystem::AddToUpQueue(WPARAM wParam)
{
	this->m_keyUpQueue.push(wParam);
}

WPARAM* InputSystem::GetKeyFromUPQueue()
{
	if (!m_keyUpQueue.empty())
	{
		WPARAM* queueKey = &m_keyUpQueue.front();
		m_keyUpQueue.pop();
		switch (*queueKey)
		{
		case VK_SHIFT:
			m_shiftMode = false;
			break;
		case VK_CONTROL:
			m_ctrlMode = false;
			break;
		case VK_CAPITAL:
			m_capsLockMode = !m_capsLockMode;
			break;
		default:
			break;
		}
		return queueKey;
	}
	else
	{
		return nullptr;
	}
}

void InputSystem::AddToDownQueue(WPARAM wParam)
{
	this->m_keyDownQueue.push(wParam);
}

void InputSystem::SetInputState(SystemState state)
{
	m_inputState = state;
	while (!m_keyDownQueue.empty())
	{
		m_keyDownQueue.pop();
	}
	while (!m_keyUpQueue.empty())
	{
		m_keyUpQueue.pop();
	}
	if (state == SystemState::TEXTFIELD)
	{
		if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0)
		{
			m_capsLockMode = true;
		}
		else
		{
			m_capsLockMode = false;
		}
	}

}

WPARAM* InputSystem::GetKeyFromDownQueue()
{
	if (!m_keyDownQueue.empty())
	{
		WPARAM* queueKey = &m_keyDownQueue.front();
		m_keyDownQueue.pop();
		switch (*queueKey)
		{
		case VK_SHIFT:
			m_shiftMode = true;
			break;
		case VK_CONTROL:
			m_ctrlMode = true;
			break;
		default:
			break;
		}
		return queueKey;
	}
	else
	{
		return nullptr;
	}
}


const bool& InputSystem::IsInShiftMode() const
{
	return m_shiftMode;
}
const bool& InputSystem::IsInCapsLock() const
{
	return m_capsLockMode;
}

const bool& InputSystem::IsInCTRLMode() const
{
	return m_ctrlMode;
}

SystemState InputSystem::GetInputState() const
{
	return m_inputState;
}

const std::unique_ptr<dx::Keyboard>& InputSystem::GetKeyboard() const
{
	return m_keyboard;
}

const std::unique_ptr<dx::Mouse>& InputSystem::GetMouse() const
{
	return m_mouse;
}

size_t InputSystem::GetDownQueueSize() const
{
	return m_keyDownQueue.size();
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

const int InputSystem::GetMouseWheelRotation()
{
	int currentValue = m_mouseState.scrollWheelValue;
	int difference = currentValue - m_lastScrollValue;
	m_lastScrollValue = currentValue;
	return difference;
}

const int InputSystem::GetMouseWheelDirection()
{
	int currentValue = m_mouseState.scrollWheelValue;
	int difference = currentValue - m_lastScrollValue;
	m_lastScrollValue = currentValue;

	if (difference < 0)
		m_lastScrollDirection = 1;
	else if (difference > 0)
		m_lastScrollDirection = -1;

	return m_lastScrollDirection;
}

int InputSystem::GetAxis(Axis axis) const
{
	int toReturn = 0;
	switch (axis)
	{
	case Axis::VERTICAL:
		if (CheckKeyboardKey(dx::Keyboard::W, KeyState::HELD) || CheckKeyboardKey(dx::Keyboard::Up, KeyState::HELD))
		{
			toReturn = -1;
		}
		else if (CheckKeyboardKey(dx::Keyboard::S, KeyState::HELD) || CheckKeyboardKey(dx::Keyboard::Down, KeyState::HELD))
		{
			toReturn = 1;
		}
		break;
	case Axis::HORIZONTAL:
		if (CheckKeyboardKey(dx::Keyboard::D, KeyState::HELD) || CheckKeyboardKey(dx::Keyboard::Right, KeyState::HELD))
		{
			toReturn = -1;
		}
		else if (CheckKeyboardKey(dx::Keyboard::A, KeyState::HELD) || CheckKeyboardKey(dx::Keyboard::Left, KeyState::HELD))
		{
			toReturn = 1;
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

std::string InputSystem::GetClipboard()
{
	if (!OpenClipboard(NULL))
	{
		LOG_WARNING("Failed to open clipboard %d", GetLastError());
		return "";
	}
	HANDLE handle = GetClipboardData(CF_TEXT);
	if (!handle)
	{
		LOG_WARNING("Failed to get clipboard handle %d", GetLastError());
		return "";
	}

	char* str = static_cast<char*>(GlobalLock(handle));
	if (!str)
	{
		LOG_WARNING("Failed to get clipboard string");
		return "";
	}

	GlobalUnlock(handle);
	CloseClipboard();

	return str;
}

void InputSystem::SetClipboard(const std::string& str)
{
	if (!OpenClipboard(NULL))
	{
		LOG_WARNING("Failed to open clipboard %d", GetLastError());
	}
	EmptyClipboard();

	HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, str.size());
	if (!hg)
	{
		CloseClipboard();
		LOG_WARNING("Failed to allocate clipboard handle %d", GetLastError());
		return;
	}

	memcpy(GlobalLock(hg), str.c_str(), str.size());
	GlobalUnlock(hg);
	SetClipboardData(CF_TEXT, hg);

	CloseClipboard();
	GlobalFree(hg);
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

		m_mouseRay.dir = sm::Vector3::TransformNormal({ viewX, viewY, 1.f }, viewInverse);
		m_mouseRay.origin = sm::Vector3::Transform({ 0.f,0.f,0.f }, viewInverse);
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
