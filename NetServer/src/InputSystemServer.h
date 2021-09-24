#pragma once

struct InputEvent
{
	UINT key_state;		// ex: WM_KEYDOWN
	WPARAM key_code;	// ex: VK_ESCAPE
};

class InputSystemServer
{
private:
	InputSystemServer() = default;

	std::queue<InputEvent> m_eventQueue;

public:
	virtual ~InputSystemServer() = default;
	
	static auto& Get()
	{
		static InputSystemServer s_instance;
		return s_instance;
	}
	
	void RegisterEvent(const UINT &uMsg, const WPARAM &wParam)
	{
		this->m_eventQueue.push({ uMsg, wParam });
	}

	bool PollEvent(InputEvent &event)
	{
		if (!m_eventQueue.empty()) {
			event = m_eventQueue.front();
			m_eventQueue.pop();
			return true;
		}
		return false;
	}

	
	// No copying allowed.
	InputSystemServer(const InputSystemServer& other) = delete;
	InputSystemServer& operator=(const InputSystemServer& other) = delete;
};
