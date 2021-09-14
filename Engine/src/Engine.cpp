#include "EnginePCH.h"
#include "Engine.h"


bool Engine::s_engineRunning = false;
bool Engine::s_safeExit = false;

Engine::Engine()
	: m_scenes({0})
	, m_currentScene(nullptr)
	, m_vSync(false)
{
}

void Engine::Setup() {
#ifdef _DEBUG
    //RedirectIoToConsole();
#endif
    
    T_INIT(T_REC, thread::ThreadType::POOL_FIFO);
    resource::ResourceManager::Initialize();
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

    m_client = std::make_unique<Client>();

}

void Engine::Start() 
{
    auto lastTime = std::chrono::high_resolution_clock::now();

    
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

        // [InputSystem Test]
        InputEvent event;
        while (InputSystem::Get().PollEvent(event))
        {
            m_currentScene->publish<InputEvent>(event);
        }
        

        auto now = std::chrono::high_resolution_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::duration<float>>(now - lastTime);
        lastTime = now;
        float dt = delta.count();
        Update(dt);
    }

    // Wait for the rendering thread to exit its last render cycle and shutdown.
    while (!s_safeExit) {};

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
    while (IsRunning())
    {
        Render();
    }
    s_safeExit = true;
}

void Engine::Update(float dt)
{
    m_frameTime.update = dt;
    // Update the camera transform based on interactive inputs.
    //updateCamera(dt);

    // Update positions, orientations and any other
    // relevant visual state of any dynamic elements
    // in the scene.

    if (m_currentScene)
    {
        m_currentScene->Update(dt);
    }
    
}

void Engine::Render()
{
    D2D1Core::Begin();
    static auto lastTime = std::chrono::high_resolution_clock::now();
    
    if (m_currentScene)
    {
        m_currentScene->Render();
    }
    auto now = std::chrono::high_resolution_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::duration<float>>(now - lastTime);
    lastTime = now;
    m_frameTime.render = delta.count();
    const std::string fps = 
        "Rendering FPS: " + std::to_string(1.0f / m_frameTime.render) +
        "\nUpdate FPS: " + std::to_string(1.0f / m_frameTime.update) +
        "\nRAM: " + std::to_string(Profiler::Get().GetRAMUsage() / (1024.f * 1024.f)) +
        "\nVRAM: " + std::to_string(Profiler::Get().GetVRAMUsage() / (1042.f * 1024.f));
    D2D1Core::DrawT(fps, &m_window);

    /*
        Present the final image and clear it for next frame.
    */
    D3D11Core::Get().SwapChain()->Present(1, 0);
    m_renderer.ClearScreen();
    D2D1Core::Present();
}

