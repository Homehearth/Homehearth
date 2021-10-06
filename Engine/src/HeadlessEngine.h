#pragma once

#include "Scene.h"
#include "EventTypes.h"

class HeadlessEngine
{
private:
	std::atomic<bool> m_isEngineRunning;
	
	std::unordered_map<std::string, Scene> m_scenes;
	Scene* m_currentScene;
	
	virtual bool OnStartup() = 0;
	virtual bool OnUserUpdate(float deltaTime) = 0;
protected:
	// Updates the current scene.
	virtual void Update(float dt);

	// Run the Engine's core loop.
	void StartUpdateLoop();

	// Shutdown the Engine and its instances in the reverse order.
	void Shutdown();

	bool IsRunning() const;

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

};

