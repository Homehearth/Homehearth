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
	std::atomic<bool> m_IsImguiReady;

	Camera m_debugCamera;
	std::shared_ptr<Camera> m_currentCamera;

	Window m_window;
	Renderer m_renderer;
	std::unique_ptr<DirectX::AudioEngine> m_audio_engine;
	std::unordered_map<std::string, Scene> m_scenes;
	Scene* m_currentScene;
	bool m_vSync;
	Client m_client;

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

	// IMGUI
	void drawImGUI() const;
};
