#pragma once
#include "Camera.h"
#include "CommonStructures.h"
enum class KeyState { PRESSED, RELEASED, HELD };
enum class MouseKey { LEFT, RIGHT, MIDDLE };
enum class Axis {VERTICAL, HORIZONTAL};
enum class SystemState {TEXTFIELD, GAME};
struct MousePos
{
	int x, y;
};
class InputSystem final
{
private:
	InputSystem();

	std::unique_ptr<dx::Keyboard> m_keyboard{};
	std::unique_ptr<dx::Mouse> m_mouse{};
	std::unique_ptr<dx::Keyboard::KeyboardStateTracker> m_kBTracker{};
	std::unique_ptr<dx::Mouse::ButtonStateTracker> m_mouseTracker{};
	std::queue<WPARAM> m_keyDownQueue;
	std::queue<WPARAM> m_keyUpQueue;
	Camera* m_currentCamera;
	dx::Keyboard::State m_kBState;
	dx::Mouse::State m_mouseState;
	SystemState m_inputState;
	MousePos m_mousePos{};
	Ray_t m_mouseRay;
	int m_windowWidth, m_windowHeight;
	int m_lastScrollValue;
	int m_lastScrollDirection;
	//Only used while in TextField State
	bool m_shiftMode, m_ctrlMode, m_capsLockMode;

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

	const bool& IsInShiftMode() const;
	const bool& IsInCapsLock() const;
	const bool& IsInCTRLMode() const;

	//CheckCollisions if keyboard keys are pressed,held or released. Enums 2nd arg: PRESSED, RELEASED or HELD
	bool CheckKeyboardKey(const dx::Keyboard::Keys& key, const KeyState state) const;
	
	void SetInputState(SystemState state);
	SystemState GetInputState() const;

	void AddToUpQueue(WPARAM wParam);
	WPARAM* GetKeyFromUPQueue();

	void AddToDownQueue(WPARAM wParam);
	WPARAM* GetKeyFromDownQueue();
	size_t GetDownQueueSize() const;

	const std::unique_ptr<dx::Keyboard>& GetKeyboard() const;
	const std::unique_ptr<dx::Mouse>& GetMouse() const;

	//CheckCollisions if mouse keys are pressed,held or released. Enums 1st arg: LEFT, MIDDLE or RIGHT. Enums 2nd arg:  PRESSED, RELEASED or HELD
	bool CheckMouseKey(const MouseKey mouseButton, const KeyState state) const;

	//Positive direction is forward towards the cabel
	const int GetMouseWheelRotation();

	//Get the rotation direction of the mouse
	const int GetMouseWheelDirection();

	//Checks if the user is holding down an axis key (WASD and Arrow keys) returns 1 on right or up, -1 on left or down (0 if nothing). Use Axis enums: V�RTICAL, HORIZONTAL
	int GetAxis(Axis axis) const;
	//Toggle the mouse mode between absolute and relative
	void SwitchMouseMode() const;

	//Toggle the visibility of the mouse. Only works in absolute mode
	void ToggleMouseVisibility() const;

	void UpdateMouseRay();

	const Ray_t& GetMouseRay() const;

	//CheckCollisions if mouse is relative
	const bool IsMouseRelative() const;

	//Get the position of the Mouse (only really works in Absolute mode)
	const MousePos& GetMousePos() const;
	// No copying allowed.
	InputSystem(const InputSystem& other) = delete;
	InputSystem& operator=(const InputSystem& other) = delete;

	std::string GetClipboard();
	void SetClipboard(const std::string& str);
};
