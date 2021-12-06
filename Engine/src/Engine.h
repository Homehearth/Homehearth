#pragma once

#include "HeadlessEngine.h"
#include "Window.h"
#include "Renderer.h"

class Engine : public BasicEngine<Scene>
{
private:
	static bool s_safeExit;
	std::mutex m_imguiMutex;
	
	std::atomic<bool> m_IsImguiReady;

	Window m_window;
	Renderer m_renderer;

	// Startup the Engine and its instances in a specific order.
	void Startup() override;

	// Job for rendering thread.
	void RenderThread();

	// Updates the current scene.
	void Update(float dt) override;
	
	// Renders one frame.
	void Render();

	// Run the Engine's core loop.
	void Run() override;

	// IMGUI
	void drawImGUI();

	virtual void OnUserUpdate(float deltaTime) = 0;

public:

	float m_masterVolume = 5.0f;
	Engine();
	Engine(const Engine& other) = delete;
	Engine(Engine&& other) = delete;
	Engine& operator=(const Engine& other) = delete;
	Engine& operator=(Engine&& other) = delete;
	virtual ~Engine() = default;
	Window* GetWindow();

};

