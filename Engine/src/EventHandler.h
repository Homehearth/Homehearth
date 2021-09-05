#pragma once

/*
 *  Note: Not yet an engine related class.
 *  TODO: Template ?
 */
class EventHandler
{
public:
    using KeyEvent = UINT;
    using KeyCode = WPARAM;
    using EventCallback = std::function<void(const unsigned char&)>;

    void RegisterCallback(const KeyEvent& event, const EventCallback& callbackFunc) {
        this->callbackKeyEvent[event].emplace_back(callbackFunc);
    }

    void Dispatch(const KeyEvent& event, const KeyCode& keycode) {
        for (const auto& callback : this->callbackKeyEvent[event]) {
            callback(static_cast<unsigned char>(keycode));
        }
    }
		
    static auto& Get() {
        static EventHandler singleton;
        return singleton;
    }

    virtual ~EventHandler() = default;

    // no copying allowed.
    EventHandler(const EventHandler& other) = delete;
    EventHandler& operator=(const EventHandler& other) = delete;
private:
    EventHandler() = default;

    std::unordered_map<KeyEvent, std::vector<EventCallback>> callbackKeyEvent;
};