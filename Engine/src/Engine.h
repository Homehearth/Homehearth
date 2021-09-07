#pragma once

#include "Window.h"
#include "Scene.h"
#include "Renderer.h"
#include "EventTypes.h"
#include "Client.h"

#include <time.h>

class Engine
{
private:
	static bool s_engineRunning;
	static bool s_safeExit;

	std::unique_ptr<Window> m_window;
	std::unique_ptr<Renderer> m_renderer;

	//CLIENT
	std::unique_ptr<Client> m_client;
	
	std::unordered_map<std::string, Scene> m_scenes;
	Scene* m_currentScene;
	bool m_vSync;

	
	void RedirectIoToConsole();

	// job for rendering thread
	void RenderThread();

	// updates the current scene
	void Update(float dt);
	// renders one frame
	void Render();

public:
	Engine();
	Engine(const Window& other) = delete;
	Engine(Window&& other) = delete;
	Engine& operator=(const Window& other) = delete;
	Engine& operator=(Window&& other) = delete;
	virtual ~Engine() = default;

	void Setup(const HINSTANCE &hInstance);

	void Start();

	void Shutdown();

	Scene& GetScene(const std::string& name);
	void SetScene(const std::string& name);
	void SetScene(Scene& scene);

	void OnEvent(EngineEvent& event);

	static bool IsRunning();

};

