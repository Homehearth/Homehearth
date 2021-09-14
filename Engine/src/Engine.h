#pragma once

#include "Window.h"
#include "Scene.h"
#include "Renderer.h"
#include "EventTypes.h"
#include "Client.h"

#include <time.h>

struct Triangle2
{
	int x;
};

class Engine
{
private:
	static bool s_engineRunning;
	static bool s_safeExit;
	thread::DoubleBuffer<std::vector<Triangle2>> m_drawBuffers;
	std::vector<Triangle2> * m_buffPointer;

	Window m_window;
	Renderer m_renderer;

	//CLIENT
	std::unique_ptr<Client> m_client;
	
	std::unordered_map<std::string, Scene> m_scenes;
	Scene* m_currentScene;
	bool m_vSync;

	struct {
		float update;
		float render;
	} m_frameTime;
	
	// job for rendering thread
	void RenderThread();

	// updates the current scene
	void Update(float dt);
	// renders one frame
	void Render(float& dt);

public:
	Engine();
	Engine(const Window& other) = delete;
	Engine(Window&& other) = delete;
	Engine& operator=(const Window& other) = delete;
	Engine& operator=(Window&& other) = delete;
	virtual ~Engine() = default;

	void Setup();

	void Start();

	void Shutdown();

	Scene& GetScene(const std::string& name);
	void SetScene(const std::string& name);
	void SetScene(Scene& scene);

	Window* GetWindow();

	void OnEvent(EngineEvent& event);

	static bool IsRunning();

};

