#pragma once

#include "Window.h"
#include "Scene.h"
#include "Renderer.h"
#include "EventTypes.h"
#include "Client.h"

#include <time.h>

// ECS DEMO
// Simple Components
struct Triangle {
	float pos[2];
	float size[2];
};
struct Velocity {
	float vel[2];
	float mag;
};

class Engine
{
private:
	static bool s_engineRunning;
	static bool s_safeExit;
	//thread::TripleBuffer<std::vector<Triangle>> m_drawBuffers;
	thread::DoubleBuffer<std::vector<Triangle>> m_drawBuffers;
	std::vector<Triangle> * pointer;
	int x = 0;
	int y = 0;
	float fps_int = 0;

	std::unique_ptr<Window> m_window;
	std::unique_ptr<Renderer> m_renderer;

	//CLIENT
	std::unique_ptr<Client> m_client;
	
	std::unordered_map<std::string, Scene> m_scenes;
	Scene* m_currentScene;
	bool m_vSync;

	struct {
		float update;
		float render;
	} m_frameTime;
	
	void RedirectIoToConsole();

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

	void Setup(const HINSTANCE &hInstance);

	void Start();

	void Shutdown();

	Scene& GetScene(const std::string& name);
	void SetScene(const std::string& name);
	void SetScene(Scene& scene);

	Window* GetWindow() const;

	void OnEvent(EngineEvent& event);

	static bool IsRunning();

};

