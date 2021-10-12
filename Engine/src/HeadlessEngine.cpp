#include "EnginePCH.h"
#include "HeadlessEngine.h"

#include <omp.h>

Scene& HeadlessEngine::GetScene(const std::string& name)
{
	return m_scenes[name];
}

Scene* HeadlessEngine::GetCurrentScene() const
{
	return m_currentScene;
}

void HeadlessEngine::SetScene(const std::string& name)
{
	SetScene(m_scenes.at(name));
}

void HeadlessEngine::SetScene(Scene& scene)
{
	if (m_currentScene)
	{
		m_currentScene->clear();
	}
	m_currentScene = &scene;

	m_currentScene->on<EShutdown>([&](const EShutdown& e, Scene& scene)
		{
			Shutdown();
		});

	m_currentScene->on<ESceneChange>([&](const ESceneChange& e, Scene& scene)
		{
			SetScene(e.newScene);
		});
}

void HeadlessEngine::Start()
{
	this->Startup();
	m_isEngineRunning = true;
	this->Run();
	this->OnShutdown();
}

void HeadlessEngine::Startup()
{
	// Sets up the game specific information
	if (!this->OnStartup())
	{
		LOG_ERROR("Failed to start game!");
		exit(0);
	}
}

HeadlessEngine::HeadlessEngine()
	: m_scenes({ 0 })
	, m_currentScene(nullptr)
{
}


void HeadlessEngine::Update(float dt)
{
	PROFILE_FUNCTION();

	this->OnUserUpdate(dt);

	// Update elements in the scene.
	if (m_currentScene)
	{
		m_currentScene->Update(dt);
	}

}

void HeadlessEngine::Run()
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
			update_time -= TARGET_UPDATE;
			Update(TARGET_UPDATE);
		}
		if (network_time >= NETWORK_TARGET_DELTA)
		{
			network_time -= NETWORK_TARGET_DELTA;
			UpdateNetwork(NETWORK_TARGET_DELTA);
		}
		network_time += deltaTime;
		update_time += deltaTime;
		lastFrame = currentFrame;
	}

}

void HeadlessEngine::Shutdown() 
{
	m_isEngineRunning = false;
}

bool HeadlessEngine::IsRunning() const
{
	return m_isEngineRunning;
}
