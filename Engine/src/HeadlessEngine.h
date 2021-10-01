#pragma once

#include "Scene.h"
#include "EventTypes.h"

class HeadlessEngine
{
private:
	std::atomic<bool> m_isEngineRunning;
	
	std::unordered_map<std::string, Scene> m_scenes;
	Scene* m_currentScene;
	
protected:
	// Updates the current scene.
	virtual void Update(float dt);

	// Run the Engine's core loop.
	virtual void Run();

	// Shutdown the Engine and its instances in the reverse order.
	virtual void Shutdown();

	bool IsRunning();

	Scene& GetScene(const std::string& name);
	Scene* GetCurrentScene() const;
	void SetScene(const std::string& name);
	void SetScene(Scene& scene);
	// Startup the Engine and its instances in a specific order.
	virtual void Startup();

public:
	HeadlessEngine();
	HeadlessEngine(const HeadlessEngine& other) = delete;
	HeadlessEngine(HeadlessEngine&& other) = delete;
	HeadlessEngine& operator=(const HeadlessEngine& other) = delete;
	HeadlessEngine& operator=(HeadlessEngine&& other) = delete;
	virtual ~HeadlessEngine() = default;

	virtual bool OnStartup() = 0;
};

