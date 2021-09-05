#pragma once

struct InputEvent {
	UINT key_state;		// ex: WM_KEYDOWN
	WPARAM key_code;	// ex: VK_ESCAPE
};

class InputSystem
{
public:
	virtual ~InputSystem() = default;
	
	static auto& Get()	{
		static InputSystem instance;
		return instance;
	}
	
	void registerEvent(const UINT &uMsg, const WPARAM &wParam) {
		this->eventQueue.push({ uMsg, wParam });
	}

	bool pollEvent(InputEvent &event) {
		if (!eventQueue.empty()) {
			event = eventQueue.front();
			eventQueue.pop();
			return true;
		}
		return false;
	}

	// no copying allowed.
	InputSystem(const InputSystem& other) = delete;
	InputSystem& operator=(const InputSystem& other) = delete;
private:
	InputSystem() = default;
	
	std::queue<InputEvent> eventQueue;
};
