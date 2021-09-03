#pragma once

#include "Window.h"
#include "Scene.h"
#include <unordered_map>

class Engine {
private:
	Window window;
	std::unordered_map<std::string, Scene> m_scenes;
	Scene* m_currentScene;
	bool m_vsync;

public:
	Engine() = default;
	virtual ~Engine() = default;

	void setup();

	void update(float dt);

	void render();

	void shutdown();
};