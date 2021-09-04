#pragma once

#include "Window.h"
#include "Scene.h"
#include <unordered_map>
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
	virtual ~Engine() = default;

	void setup();

	void update(float dt);

	void render();

	void shutdown();
};

