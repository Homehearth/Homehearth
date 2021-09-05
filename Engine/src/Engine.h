#pragma once

#include "Window.h"
#include "Scene.h"
#include "Renderer.h"

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
	std::unique_ptr<Window> window;
	std::unique_ptr<Renderer> renderer;
	
	std::unordered_map<std::string, Scene> m_scenes;
	Scene* m_currentScene;
	bool m_vSync;
	bool m_isOn;

	void RedirectIoToConsole();
};

