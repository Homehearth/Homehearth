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
	void SetUpdateRate(float rate);
	
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
	m_currentScene->publish<ESceneStart>();
}

template<typename SceneType>
inline void BasicEngine<SceneType>::SetUpdateRate(float rate)
{
	Stats::Get().SetUpdateRate(rate);
}

template<typename SceneType>
void BasicEngine<SceneType>::Start()
{
	m_isEngineRunning = true;
	this->Startup();
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
	const float TARGET_UPDATE	= 1.f / Stats::Get().GetUpdaterate();
	const float TICK_RATE		= 1.f / Stats::Get().GetTickrate();

	while (IsRunning())
	{
		PROFILE_SCOPE("Update Frame");
		// Update time.
		currentFrame = omp_get_wtime();
		deltaTime = static_cast<float>(currentFrame - lastFrame);

		update_time += deltaTime;
		if (update_time >= TARGET_UPDATE)
		{
			Stats::Get().SetUpdateTime(update_time);
			Update(update_time);
			update_time = 0.f;
		}

		network_time += deltaTime;
		if (network_time >= TICK_RATE)
		{
			Stats::Get().SetNetworkTime(network_time);
			UpdateNetwork(network_time);
			network_time = 0.f;
		}

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