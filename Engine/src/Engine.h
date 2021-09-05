#pragma once

#include "Window.h"
#include "D3D11Core.h"
#include "Scene.h"

class Engine
{
public:
	Engine();
	Engine(const Window& other) = delete;
	Engine(Window&& other) = delete;
	Engine& operator=(const Window& other) = delete;
	Engine& operator=(Window&& other) = delete;
	virtual ~Engine() = default;

	void setup(const HINSTANCE &hInstance);

	void update(float dt);

	void render();

	void shutdown();

private:
	Window window;
	D3D11Core dxCore;

	
	std::unordered_map<std::string, Scene> m_scenes;
	Scene* m_currentScene;
	bool m_vsync;
	bool m_isOn;

	void RedirectIoToConsole();
};

