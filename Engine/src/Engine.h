#pragma once

#include "HeadlessEngine.h"
#include "Window.h"
#include "Renderer.h"

class Engine : public HeadlessEngine
{
private:
	static bool s_safeExit;
	std::mutex m_imguiMutex;
	
	std::atomic<bool> m_IsImguiReady;

	Camera m_gameCamera;
	Camera m_debugCamera;
	std::shared_ptr<Camera> m_currentCamera;
	bool isDebug;

	Window m_window;
	Renderer m_renderer;
	std::unique_ptr<DirectX::AudioEngine> m_audio_engine;
	
	struct {
		float update;
		float render;
	} m_frameTime;

	// Job for rendering thread.
	void RenderThread();

	// Updates the current scene.
	void Update(float dt) override;
	
	// Renders one frame.
	void Render(float& dt);

	// Run the Engine's core loop.
	void Run() override;

	Window* GetWindow();

	// IMGUI
	void drawImGUI() const;
	void CameraUpdate(float deltaTime);

protected:
	// Startup the Engine and its instances in a specific order.
	void Startup() override;

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