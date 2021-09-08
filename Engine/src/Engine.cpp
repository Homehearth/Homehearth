#include "EnginePCH.h"
#include "Engine.h"
#include "multi_thread_manager.h"
#include <chrono>
#include <functional>
#include <omp.h>

bool Engine::s_engineRunning = false;
bool Engine::s_safeExit = false;

Engine::Engine()
	: m_window(std::make_unique<Window>())
	, m_scenes({0})
	, m_currentScene(nullptr)
	, m_vSync(false)
{
}

void Engine::Setup(const HINSTANCE& hInstance) {
#ifdef _DEBUG
    //RedirectIoToConsole();
#endif
    RedirectIoToConsole();

    T_INIT(T_REC, thread::ThreadType::POOL_FIFO);
    resource::ResourceManager::Initialize();
    srand((unsigned int)time(NULL));

    // Window Setup:
    Window::Desc config;
    config.hInstance = hInstance;
    config.title = L"Engine Window";
    if (!m_window->Initialize(config)) {
        LOG_ERROR("Could not Initialize m_window.");
    }

    // DirectX Setup:
    D3D11Core::Get().Initialize(this->m_window.get());
    D2D1Core::Initialize(this->m_window.get());
    m_renderer = std::make_unique<Renderer>();
    m_renderer->initialize(m_window.get());
    // Thread should be launched after s_engineRunning is set to true and D3D11 is initalized.
    s_engineRunning = true;
    this->t.Setup(100000000);

    m_client = std::make_unique<Client>();

    // register Engine OnEvent function so Scene can talk to Engine
    Scene::GetEventDispatcher().sink<EngineEvent>().connect<&Engine::OnEvent>(this);

}

void Engine::Start() 
{
    auto lastTime = std::chrono::high_resolution_clock::now();

    if (thread::IsThreadActive())
        T_CJOB(Engine, RenderThread);

    double currentFrame = 0.f, lastFrame = omp_get_wtime();
    float deltaTime = 0.f, deltaSum = 0.f;
    // Desired FPS
    const float targetDelta = 1 / 1000.f;
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

        currentFrame = omp_get_wtime();
        deltaTime = static_cast<float>(currentFrame - lastFrame);
        if (deltaSum >= targetDelta)
        {
            // [InputSystem Test]
            InputEvent event;
            while (InputSystem::Get().PollEvent(event))
            {
                Scene::GetEventDispatcher().enqueue<InputEvent>(event);
            }

            fps_int = deltaSum;
            Update(deltaSum);
            deltaSum = 0.f;
        }
        deltaSum += deltaTime;
        lastFrame = currentFrame;

       
    }
}

void Engine::Shutdown() 
{
    s_engineRunning = false;
    // Wait for the rendering thread to exit its last render cycle and shutdown.
    while (!s_safeExit) {};

    T_DESTROY();
    resource::ResourceManager::Destroy();
    D2D1Core::Destroy();
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
    m_currentScene = &scene;
}

void Engine::OnEvent(EngineEvent& event) {
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

void Engine::RedirectIoToConsole()
{
    AllocConsole();
    HANDLE stdHandle;
    int hConsole;
    FILE* fp;
    stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    hConsole = _open_osfhandle((intptr_t)stdHandle, _O_TEXT);
    fp = _fdopen(hConsole, "w");
    freopen_s(&fp, "CONOUT$", "w", stdout);
}

void Engine::RenderThread()
{
    double currentFrame = 0.f, lastFrame = omp_get_wtime();
    float deltaTime = 0.f, deltaSum = 0.f;
    // Desired FPS
    const float targetDelta = 1 / 144.f;
    float fps = 0;
    while (IsRunning())
    {
        currentFrame = omp_get_wtime();
        deltaTime = static_cast<float>(currentFrame - lastFrame);
        if (deltaSum >= targetDelta)
        {
            fps = deltaSum;
            Render(fps);
            deltaSum = 0.f;
        }
        deltaSum += deltaTime;
        lastFrame = currentFrame;
    }

    s_safeExit = true;
}

void Engine::Update(float dt)
{
    // Update the camera transform based on interactive inputs.
    //updateCamera(dt);

    // Update positions, orientations and any other
    // relevant visual state of any dynamic elements
    // in the scene.

    if (m_currentScene)
    {
        m_currentScene->Update(dt);
    }

    for (int i = 0; i < 1000000; i++)
    {
        int temp = 500000;
        t.buffer[i] = &temp;
    }


    //m_drawBuffers.SetUpBuffer(0, std::move(t));
    m_drawBuffers.SwapBuffers(0, 1);
    //std::cout << "Y: " << y++ << "\n";
    // Handle events enqueued
    Scene::GetEventDispatcher().update();
}

void Engine::Render(float& dt)
{
    D2D1Core::Begin();
   // m_drawBuffers.SwapBuffers(1, 2);
    //for (int i = 0; i < 10000; i++)
        //D2D1Core::DrawF(0, 0, 100, 100, Shapes::RECTANGLE_FILLED);
          //D2D1Core::DrawF(rand() % m_window.get()->GetWidth() - 100, rand() % m_window.get()->GetHeight() - 100, 100, 100, Shapes::RECTANGLE_FILLED);
    if (m_currentScene)
    {
        m_currentScene->Render();
    }

    thread::Buff* readBlock = nullptr;
    //m_drawBuffers.GetBuffer(2, readBlock);
    if (readBlock)
    {
        x++;
        //std::cout << "X: " << x << "\n";
    }

    const std::string fps = "Update FPS: " + std::to_string(1.0f / fps_int)
        + "\nRender FPS: " + std::to_string(1.0f / dt)
        + "\nRAM: " + std::to_string(Profiler::Get().GetRAMUsage() / (1024.f * 1024.f))
        + "\nVRAM: " + std::to_string(Profiler::Get().GetVRAMUsage() / (1042.f * 1024.f));
    D2D1Core::DrawT(fps, m_window.get());

    /*
        Present the final image and clear it for next frame.
    */

    D3D11Core::Get().SwapChain()->Present(0, 0);
    m_renderer.get()->clearScreen();
    D2D1Core::Present();

}

