#pragma once

#include "Window.h"
#include "Scene.h"
#include "Renderer.h"
#include "EventTypes.h"
#include "Client.h"

class Engine
{
private:
	static bool s_engineRunning;
	static bool s_safeExit;
	std::mutex m_imguiMutex;
	
	Window m_window;
	Renderer m_renderer;
	std::unique_ptr<DirectX::AudioEngine> m_audio_engine;
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

	// Run the Engine's core loop.
	void Run();

	// Shutdown the Engine and its instances in the reverse order.
	void Shutdown();

	Window* GetWindow();

	static bool IsRunning();

	// IMGUI
	void drawImGUI() const;

protected:
	Scene& GetScene(const std::string& name);
	void SetScene(const std::string& name);
	void SetScene(Scene& scene);
	// Startup the Engine and its instances in a specific order.
	void Startup();

public:
	Engine();
	Engine(const Engine& other) = delete;
	Engine(Engine&& other) = delete;
	Engine& operator=(const Engine& other) = delete;
	Engine& operator=(Engine&& other) = delete;
	virtual ~Engine() = default;

	virtual bool OnStartup() = 0;
	virtual bool OnUserUpdate(float deltaTime) = 0;
};