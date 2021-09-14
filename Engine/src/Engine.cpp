﻿#include "EnginePCH.h"
#include "Engine.h"
#include <omp.h>


bool Engine::s_engineRunning = false;
bool Engine::s_safeExit = false;

Engine::Engine()
	: m_scenes({0})
	, m_currentScene(nullptr)
	, m_vSync(false)
    , m_frameTime()
    , m_buffPointer(nullptr)
{
	LOG_INFO("Engine(): " __TIMESTAMP__);
}

void Engine::Startup()
{
    T_INIT(1, thread::ThreadType::POOL_FIFO);
    resource::ResourceManager::Initialize();
    srand((unsigned int)time(NULL));

	// Window Startup:
	Window::Desc config;
	config.title = L"Engine";
	if (!m_window.Initialize(config)) 
    {
		LOG_ERROR("Could not Initialize m_window.");
	}
	
    // DirectX Startup:
    D3D11Core::Get().Initialize(&m_window);
    D2D1Core::Initialize(&m_window);
    
    m_renderer.Initialize(&m_window);

    // Thread should be launched after s_engineRunning is set to true and D3D11 is initalized.
    s_engineRunning = true;

    // Preallocate space for Triplebuffer
    m_drawBuffers.AllocateBuffers();
    m_buffPointer = m_drawBuffers.GetBuffer(0);
    m_buffPointer->reserve(200);
    m_buffPointer = m_drawBuffers.GetBuffer(1);
    m_buffPointer->reserve(200);

    //m_client = std::make_unique<Client>();

}

void Engine::Run() 
{
    double currentFrame = 0.f;
	double lastFrame = omp_get_wtime();
    float deltaTime = 0.f;
	float accumulator = 0.f;
    const float targetDelta = 1 / 1000.0f;

    if (thread::IsThreadActive())
        T_CJOB(Engine, RenderThread);

	
    MSG msg = { nullptr };
    while (IsRunning())
    {
        // Service any and all pending Windows messages.
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
            {
                Shutdown();
            }
        }
    	
		// Handle Input.
        // todo:


    	
    	// Update time.
        currentFrame = omp_get_wtime();
        deltaTime = static_cast<float>(currentFrame - lastFrame);

    	
        if (accumulator >= targetDelta)
        {
            Update(accumulator);

            m_frameTime.update = accumulator;
            accumulator = 0.f;
        }
        accumulator += deltaTime;
        lastFrame = currentFrame;	
    }

    // Wait for the rendering thread to exit its last render cycle and shutdown.
#ifdef _DEBUG
    while (!s_safeExit) {};
#endif

    T_DESTROY();
    resource::ResourceManager::Destroy();
    D2D1Core::Destroy();
}

void Engine::Shutdown() 
{
    s_engineRunning = false;
}

Scene& Engine::GetScene(const std::string& name) 
{
    return m_scenes[name];
}

void Engine::SetScene(const std::string& name) 
{
    SetScene(m_scenes.at(name));
}

void Engine::SetScene(Scene& scene)
{
    if (m_currentScene)
    {
        m_currentScene->clear();
    }
    m_currentScene = &scene;
    m_currentScene->on<EngineEvent>([&](const EngineEvent& e, Scene& scene) 
        {
            switch (e.type)
            {
            case EngineEvent::Type::SHUTDOWN:
                Shutdown();
                break;
            default:
                break;
            }
        });
}

Window* Engine::GetWindow()
{
    return &m_window;
}

void Engine::OnEvent(EngineEvent& event) 
{
    switch (event.type)
    {
    case EngineEvent::Type::SHUTDOWN:
        Shutdown();
        break;
    default:
        break;
    }
}

bool Engine::IsRunning() 
{
    return s_engineRunning;
}

void Engine::RenderThread()
{
    double currentFrame = 0.f, lastFrame = omp_get_wtime();
    float deltaTime = 0.f, deltaSum = 0.f;
    // Desired FPS
    const float targetDelta = 1 / 144.01f;
    while (IsRunning())
    {
        currentFrame = omp_get_wtime();
        deltaTime = static_cast<float>(currentFrame - lastFrame);
        if (deltaSum >= targetDelta)
        {
            if (m_drawBuffers.IsSwapped())
            {
                Render(deltaSum);
            }

            m_frameTime.render = deltaSum;
            deltaSum = 0.f;
        }
        deltaSum += deltaTime;
        lastFrame = currentFrame;
        
    }

    s_safeExit = true;
}

void Engine::Update(float dt)
{
    m_buffPointer = m_drawBuffers.GetBuffer(0);
    // Update the camera transform based on interactive inputs.
    //updateCamera(dt);

    // Update positions, orientations and any other
    // relevant visual state of any dynamic elements
    // in the scene.

    if (m_currentScene)
    {
        m_currentScene->Update(dt);
    }
    
    //std::cout << "Y: " << y++ << "\n";
    // Handle events enqueued
    //Scene::GetEventDispatcher().update();

    if (!m_drawBuffers.IsSwapped())
    {
        m_drawBuffers.SwapBuffers();
    }
}

void Engine::Render(float& dt)
{
    m_renderer.ClearScreen();
    D2D1Core::Begin();
    if (m_currentScene)
    {
        m_currentScene->Render();
    }

    const std::string fps = "Render FPS: " + std::to_string(1.0f / m_frameTime.render)
        + "\nUpdate FPS: " + std::to_string(1.0f / m_frameTime.update)
        + "\nRAM: " + std::to_string(Profiler::Get().GetRAMUsage() / (1024.f * 1024.f)) + " MB"
        + "\nVRAM: " + std::to_string(Profiler::Get().GetVRAMUsage() / (1042.f * 1024.f)) + " MB";
    D2D1Core::DrawT(fps, &m_window);

    /*
        Present the final image and clear it for next frame.
    */

    /*
        Kanske v�nta p� att uppdateringstr�den kan swappa buffrar.
    */
    m_drawBuffers.ReadySwap();
    D2D1Core::Present();
    D3D11Core::Get().SwapChain()->Present(1, 0);
    m_renderer.ClearScreen();
}

