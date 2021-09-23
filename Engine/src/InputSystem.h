#pragma once
struct InputEvent
{
	UINT key_state;		// ex: WM_KEYDOWN
	WPARAM key_code;	// ex: VK_ESCAPE
};
enum class KeyState { PRESSED, RELEASED, HELD };
enum class MouseKey { LEFT, RIGHT, MIDDLE };
struct MousePos
{
	float x, y;
	MousePos(float xPos, float yPos)
	{
		x = xPos;
		y = yPos;
	}
};
class InputSystem
{
private:
	InputSystem();

	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	std::unique_ptr<DirectX::Mouse> m_mouse;
	std::unique_ptr<DirectX::Keyboard::KeyboardStateTracker> m_kBTracker;
	std::unique_ptr<DirectX::Mouse::ButtonStateTracker> m_mouseTracker;
	DirectX::Keyboard::State m_kBState;
	DirectX::Mouse::State m_mouseState;

public:
	virtual ~InputSystem() = default;
	//Set which window for the mouse to operate in
	void SetMouseWindow(const HWND& windowHandle);

	//Updates KB and Mouse, checking new inputs
	void UpdateEvents();

	//Check if keyboard keys are pressed,held or released. Enums 2nd arg: PRESSED, RELEASED or HELD
	const bool CheckKeyboardKey(const DirectX::Keyboard::Keys& key, const KeyState state);

	const std::unique_ptr<DirectX::Keyboard>& GetKeyboard() const;
	const std::unique_ptr<DirectX::Mouse>& GetMouse() const;

	//Check if mouse keys are pressed,held or released. Enums 1st arg: LEFT, MIDDLE or RIGHT. Enums 2nd arg:  PRESSED, RELEASED or HELD
	const bool CheckMouseKey(const MouseKey mouseButton, const KeyState state);

	//Toggle the mouse mode between absolute and relative
	void SwitchMouseMode();

	//Toggle the visibility of the mouse. Only works in absolute mode
	void ToggleMouseVisibility();

	//Get the position of the Mouse (only really works in Absolute mode)
	const MousePos& GetMousePos() const;
	// No copying allowed.
	InputSystem(const InputSystem& other) = delete;
	InputSystem& operator=(const InputSystem& other) = delete;
};