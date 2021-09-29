#pragma once
#include "Camera.h"
enum class KeyState { PRESSED, RELEASED, HELD };
enum class MouseKey { LEFT, RIGHT, MIDDLE };
enum class Axis {VERTICAL, HORIZONTAL};
struct MousePos
{
	int x, y;
};
struct MouseRay
{
	sm::Vector3 rayPos, rayDir;
};
class InputSystem
{
private:
	InputSystem();

	std::unique_ptr<dx::Keyboard> m_keyboard;
	std::unique_ptr<dx::Mouse> m_mouse;
	std::unique_ptr<dx::Keyboard::KeyboardStateTracker> m_kBTracker;
	std::unique_ptr<dx::Mouse::ButtonStateTracker> m_mouseTracker;
	Camera* m_currentCamera;
	dx::Keyboard::State m_kBState;
	dx::Mouse::State m_mouseState;
	MousePos m_mousePos;
	MouseRay m_mouseRay;
	int m_windowWidth, m_windowHeight;

public:
	virtual ~InputSystem() = default;
	static auto& Get()
	{
		static InputSystem s_instance;
		return s_instance;
	}

	//Set which window for the mouse to operate in
	void SetMouseWindow(const HWND& windowHandle, const int width, const int height);
	//Set which camera to operate in
	void SetCamera(Camera* camera);
	//Updates KB and Mouse, checking new inputs
	void UpdateEvents();

	//Check if keyboard keys are pressed,held or released. Enums 2nd arg: PRESSED, RELEASED or HELD
	const bool CheckKeyboardKey(const dx::Keyboard::Keys& key, const KeyState state) const;

	const std::unique_ptr<dx::Keyboard>& GetKeyboard() const;
	const std::unique_ptr<dx::Mouse>& GetMouse() const;

	//Check if mouse keys are pressed,held or released. Enums 1st arg: LEFT, MIDDLE or RIGHT. Enums 2nd arg:  PRESSED, RELEASED or HELD
	const bool CheckMouseKey(const MouseKey mouseButton, const KeyState state) const;

	//Checks if the user is holding down an axis key (WASD and Arrow keys) returns 1 on right or up, -1 on left or down (0 if nothing). Use Axis enums: V�RTICAL, HORIZONTAL
	const int GetAxis(Axis axis) const;
	//Toggle the mouse mode between absolute and relative
	void SwitchMouseMode();

	//Toggle the visibility of the mouse. Only works in absolute mode
	void ToggleMouseVisibility();

	void UpdateMouseRay();

	const MouseRay GetMouseRay() const;

	//Get the position of the Mouse (only really works in Absolute mode)
	const MousePos GetMousePos() const;
	// No copying allowed.
	InputSystem(const InputSystem& other) = delete;
	InputSystem& operator=(const InputSystem& other) = delete;
};
