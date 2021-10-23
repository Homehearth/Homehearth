#pragma once

#include "Scene.h"

#include <omp.h>

template<typename SceneType>
class BasicEngine
{
private:
	std::atomic<bool> m_isEngineRunning;
	
	std::unordered_map<std::string, SceneType> m_scenes;
	SceneType* m_currentScene;

	virtual void UpdateNetwork(float deltaTime) = 0;
	virtual bool OnStartup() = 0;
	virtual void OnShutdown() = 0;
	
protected:
	typedef SceneType scene_t;
	// Startup the Engine and its instances in a specific order.
	virtual void Startup();

	// Run the Engine's core loop.
	virtual void Run();

	// Updates the current scene.
	virtual void Update(float dt);

public:
	BasicEngine();
	BasicEngine(const BasicEngine& other) = delete;
	BasicEngine(BasicEngine&& other) = delete;
	BasicEngine& operator=(const BasicEngine& other) = delete;
	BasicEngine& operator=(BasicEngine&& other) = delete;
	virtual ~BasicEngine() = default;

	bool IsRunning() const;

	SceneType& GetScene(const std::string& name);
	SceneType* GetCurrentScene() const;
	void SetScene(const std::string& name);
	void SetScene(SceneType& scene);
	
	void Start();

	void Shutdown();
};

template<typename SceneType>
SceneType& BasicEngine<SceneType>::GetScene(const std::string& name)
{
	return m_scenes[name];
}

template<typename SceneType>
SceneType* BasicEngine<SceneType>::GetCurrentScene() const
{
	return m_currentScene;
}

template<typename SceneType>
void BasicEngine<SceneType>::SetScene(const std::string& name)
{
	SetScene(GetScene(name));
}

template<typename SceneType>
void BasicEngine<SceneType>::SetScene(SceneType& scene)
{	
	m_currentScene = &scene;
}

template<typename SceneType>
void BasicEngine<SceneType>::Start()
{
	this->Startup();
	m_isEngineRunning = true;
	this->Run();
	this->OnShutdown();
}

template<typename SceneType>
void BasicEngine<SceneType>::Startup()
{
	// Sets up the game specific information
	if (!this->OnStartup())
	{
		LOG_ERROR("Failed to start game!");
		exit(0);
	}
}

template<typename SceneType>
BasicEngine<SceneType>::BasicEngine()
	: m_scenes({ 0 })
	, m_currentScene(nullptr)
{
}

template<typename SceneType>
void BasicEngine<SceneType>::Update(float dt)
{
	PROFILE_FUNCTION();

	// Update elements in the scene.
	if (m_currentScene)
	{
		m_currentScene->Update(dt);
	}

}

template<typename SceneType>
void BasicEngine<SceneType>::Run()
{
	double currentFrame = 0.f;
	double lastFrame = omp_get_wtime();
	float deltaTime = 0.f;
	float update_time = 0.f;
	float network_time = 0.f;
	const float TARGET_UPDATE = 1.f / 144.f;
	const float NETWORK_TARGET_DELTA = 1.f / 60.f;

	while (IsRunning())
	{
		PROFILE_SCOPE("Update Frame");
		// Update time.
		currentFrame = omp_get_wtime();
		deltaTime = static_cast<float>(currentFrame - lastFrame);

		if (update_time >= TARGET_UPDATE)
		{
			Update(TARGET_UPDATE);
			update_time -= TARGET_UPDATE;
		}
		if (network_time >= NETWORK_TARGET_DELTA)
		{
			UpdateNetwork(NETWORK_TARGET_DELTA);
			network_time -= NETWORK_TARGET_DELTA;
		}
		network_time += deltaTime;
		update_time += deltaTime;
		lastFrame = currentFrame;
	}
}

template<typename SceneType>
void BasicEngine<SceneType>::Shutdown()
{
	m_isEngineRunning = false;
}

template<typename SceneType>
bool BasicEngine<SceneType>::IsRunning() const
{
	return m_isEngineRunning;
}


typedef BasicEngine<HeadlessScene> HeadlessEngine;