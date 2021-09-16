#include "EnginePCH.h"
#include "Engine.h"
#include <omp.h>
#include "BackBuffer.h"

#include "RMesh.h"

bool Engine::s_engineRunning = false;
bool Engine::s_safeExit = false;

Engine::Engine()
	: m_scenes({0})
	, m_currentScene(nullptr)
	, m_vSync(false)
    , m_frameTime()
{
}

void Engine::Setup() {
#ifdef _DEBUG
    //RedirectIoToConsole();
#endif
    //RedirectIoToConsole();

    T_INIT(1, thread::ThreadType::POOL_FIFO);
    ResourceManager::Initialize();
    //Backbuffer::Initialize();
    srand((unsigned int)time(NULL));

	// Window Setup:
	Window::Desc config;
	config.title = L"Engine Window";
	if (!m_window.Initialize(config)) {
		LOG_ERROR("Could not Initialize m_window.");
	}
	
    // DirectX Setup:
    D3D11Core::Get().Initialize(&m_window);
    D2D1Core::Initialize(&m_window);
    
    m_renderer.Initialize(&m_window);

    // Thread should be launched after s_engineRunning is set to true and D3D11 is initalized.
    s_engineRunning = true;

    // *** [TEMP] testing to load in a mesh ***
    ResourceManager::GetResource<RMesh>("Monster.fbx");

    //m_client = std::make_unique<Client>();

}

void Engine::Start() 
{
    auto lastTime = std::chrono::high_resolution_clock::now();

    
    if (thread::IsThreadActive())
        T_CJOB(Engine, RenderThread);

    double currentFrame = 0.f, lastFrame = omp_get_wtime();
    float deltaTime = 0.f, deltaSum = 0.f;
    // Desired FPS
    const float targetDelta = 1 / 1000.0f;
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

        // [InputSystem Test]
        InputEvent event;
        while (InputSystem::Get().PollEvent(event))
        {
            m_currentScene->publish<InputEvent>(event);
        }


        currentFrame = omp_get_wtime();
        deltaTime = static_cast<float>(currentFrame - lastFrame);
        if (deltaSum >= targetDelta)
        {
            Update(deltaSum);

            m_frameTime.update = deltaSum;
            deltaSum = 0.f;
        }
        deltaSum += deltaTime;
        lastFrame = currentFrame;
    }

    // Wait for the rendering thread to exit its last render cycle and shutdown.
#ifdef _DEBUG
    while (!s_safeExit) {};
#endif

    T_DESTROY();
    ResourceManager::Destroy();
    //Backbuffer::Destroy();
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
            /*
            if (Backbuffer::m_buffers.IsSwapped())
            {
                Render(deltaSum);
            }
            */

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
    //Backbuffer::m_bPointer = Backbuffer::m_buffers.GetBuffer(0);
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

    /*
    if (Backbuffer::m_buffers.IsSwapped())
    {
        Backbuffer::m_buffers.SwapBuffers();
    }
    */
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
    //Backbuffer::m_buffers.ReadySwap();
    D2D1Core::Present();
    D3D11Core::Get().SwapChain()->Present(1, 0);
    m_renderer.ClearScreen();
}

