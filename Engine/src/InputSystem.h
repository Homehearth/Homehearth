#pragma once
#include <EnginePCH.h>
#pragma warning(push, 0)
#include <Keyboard.h>
#include <Mouse.h>
#pragma warning(pop)
struct InputEvent
{
	UINT key_state;		// ex: WM_KEYDOWN
	WPARAM key_code;	// ex: VK_ESCAPE
};
enum class KeyState {PRESSED, RELEASED, HELD};
enum class MouseKey {LEFT, RIGHT, MIDDLE};
class InputSystem
{
private:
	InputSystem();

	//Unsure if needed
	std::queue<InputEvent> m_eventQueue;

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
	static InputSystem& Get()
	{
		static InputSystem s_instance;
		return s_instance;
	}
	
	//Add an event to the queue
	void RegisterEvent(const UINT &uMsg, const WPARAM &wParam);

	//Pull potential events from the queue
	bool PollEvent(InputEvent& event);

	const std::unique_ptr<DirectX::Keyboard>& GetKeyboard() const;

	//Updates KB and Mouse, checking new inputs
	void UpdateEvents();

	//Check if keyboard keys are pressed,held or released. Enums 2nd arg: PRESSED, RELEASED or HELD
	const bool CheckKeyboardKey(const DirectX::Keyboard::Keys& key, const KeyState state);

	//Check if mouse keys are pressed,held or released. Enums 1st arg: LEFT, MIDDLE or RIGHT. Enums 2nd arg:  PRESSED, RELEASED or HELD
	const bool CheckMouseKey(const MouseKey mouseButton, const KeyState state);

	// No copying allowed.
	InputSystem(const InputSystem& other) = delete;
	InputSystem& operator=(const InputSystem& other) = delete;
};
