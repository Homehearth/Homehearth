#pragma once

#include "Window.h"
#include "Scene.h"
#include "Renderer.h"
#include "EventTypes.h"

struct Triangle2
{
	int x;
};

class Engine
{
private:
	static bool s_engineRunning;
	static bool s_safeExit;
	std::atomic<bool> m_IsImguiReady;
	thread::DoubleBuffer<std::vector<Triangle2>> m_drawBuffers;
	std::vector<Triangle2>* m_buffPointer;

	Window m_window;
	Renderer m_renderer;
	
	std::unordered_map<std::string, Scene> m_scenes;
	Scene* m_currentScene;
	bool m_vSync;

	struct {
		float update;
		float render;
	} m_frameTime;
	
	// Job for rendering thread.
	void RenderThread();

	// Updates the current scene.
	void Update(float dt);
	// Renders one frame.
	void Render(float& dt);


public:
	Engine();
	Engine(const Window& other) = delete;
	Engine(Window&& other) = delete;
	Engine& operator=(const Window& other) = delete;
	Engine& operator=(Window&& other) = delete;
	virtual ~Engine() = default;

	// Startup the Engine and its instances in a specific order.
	void Startup();

	// Run the Engine's core loop.
	void Run();

	// Shutdown the Engine and its instances in the reverse order.
	void Shutdown();

	Scene& GetScene(const std::string& name);
	void SetScene(const std::string& name);
	void SetScene(Scene& scene);

	Window* GetWindow();

	static bool IsRunning();

	//IMGUI
	void drawImGUI() const;
};

