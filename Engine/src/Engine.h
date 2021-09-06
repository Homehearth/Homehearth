#pragma once

#include "Window.h"
#include "Scene.h"
#include "Renderer.h"
#include "Client.h"

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

	static bool engineRunning;
private:
	std::unique_ptr<Window> window;
	std::unique_ptr<Renderer> renderer;

	//CLIENT
	std::unique_ptr<Client> m_client;
	
	std::unordered_map<std::string, Scene> m_scenes;
	Scene* m_currentScene;
	bool m_vSync;

	void RedirectIoToConsole();
};

