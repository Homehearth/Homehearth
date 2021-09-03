#pragma once

#include "Scene.h"
#include <unordered_map>

class Engine {
private:
	std::unordered_map<std::string, Scene> m_scenes;
	Scene* m_currentScene;

	bool m_vsync;

public:
	Engine();
	virtual ~Engine();

	void setup();

	void update(float dt);

	void render();

};