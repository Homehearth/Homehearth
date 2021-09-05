#pragma once

#include "Window.h"
#include "Scene.h"
#include "Buffers.h"

class Engine {
private:
	Window window;
	std::unordered_map<std::string, Scene> m_scenes;
	Scene* m_currentScene;
	bool m_vsync;

	void RedirectIoToConsole();
public:
	Engine();
	Engine(const Window& other) = delete;
	Engine(Window&& other) = delete;
	Engine& operator=(const Window& other) = delete;
	Engine& operator=(Window&& other) = delete;
	virtual ~Engine() = default;

	void setup();

	void update(float dt);

	void render();

	void shutdown();

private:
	Window window;
	std::unordered_map<std::string, Scene> m_scenes;
	Scene* m_currentScene;
	bool m_vsync;
	bool m_isOn;

	void RedirectIoToConsole();
};

