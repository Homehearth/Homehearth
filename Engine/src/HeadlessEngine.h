#pragma once

#include "Scene.h"

class HeadlessEngine
{
private:
	std::atomic<bool> m_isEngineRunning;
	
	std::unordered_map<std::string, Scene> m_scenes;
	Scene* m_currentScene;

	virtual void UpdateNetwork(float deltaTime) = 0;
	virtual bool OnStartup() = 0;
	virtual void OnUserUpdate(float deltaTime) = 0;
	virtual void OnShutdown() = 0;
	
protected:

	// Startup the Engine and its instances in a specific order.
	virtual void Startup();

	// Run the Engine's core loop.
	virtual void Run();

	// Updates the current scene.
	virtual void Update(float dt);

public:
	HeadlessEngine();
	HeadlessEngine(const HeadlessEngine& other) = delete;
	HeadlessEngine(HeadlessEngine&& other) = delete;
	HeadlessEngine& operator=(const HeadlessEngine& other) = delete;
	HeadlessEngine& operator=(HeadlessEngine&& other) = delete;
	virtual ~HeadlessEngine() = default;

	bool IsRunning() const;

	Scene& GetScene(const std::string& name);
	Scene* GetCurrentScene() const;
	void SetScene(const std::string& name);
	void SetScene(Scene& scene);

	void Start();

	void Shutdown();
};

