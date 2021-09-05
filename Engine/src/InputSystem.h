#pragma once

struct InputEvent {
	UINT key_state;
	WPARAM key_code;
};

class InputSystem
{
public:	
	static auto& Get()	{
		static InputSystem instance;
		return instance;
	}
	
	void addEvent(const UINT &uMsg, const WPARAM &wParam) {
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
	
	virtual ~InputSystem() = default;

	// no copying allowed.
	InputSystem(const InputSystem& other) = delete;
	InputSystem& operator=(const InputSystem& other) = delete;
private:
	InputSystem() = default;
	
	std::queue<InputEvent> eventQueue;
};
