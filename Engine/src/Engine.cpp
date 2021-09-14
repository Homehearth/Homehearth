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

    T_INIT(1, thread::ThreadType::POOL_FIFO);
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

    /*
        Preallocate space for Triplebuffer
    */
    m_drawBuffers.AllocateBuffers();
    this->pointer = m_drawBuffers.GetBuffer(0);
    this->pointer->reserve(200);
    this->pointer = m_drawBuffers.GetBuffer(1);
    this->pointer->reserve(200);

    m_client = std::make_unique<Client>();

    // register Engine OnEvent function so Scene can talk to Engine
    Scene::GetEventDispatcher().sink<EngineEvent>().connect<&Engine::OnEvent>(this);




    Scene& scene = GetScene("StartScene");
    // System to update components
    scene.AddSystem([&](entt::registry& reg, float dt)
    {
        std::vector<Triangle>* pointer = nullptr;
        // Get the buffer.
        pointer = m_drawBuffers.GetBuffer(0);
        if (pointer)
        {
            // Clear list if not already done.
            if(!pointer->empty())
                pointer->clear();

            auto view = reg.view<Triangle, Velocity>();
            view.each([&, dt](Triangle& triangle, Velocity& velocity)
            {
                if (triangle.pos[0] > GetWindow()->GetWidth() - triangle.size[0] ||
                    triangle.pos[0] < 0)
                {
                    velocity.vel[0] = -velocity.vel[0];
                }
                if (triangle.pos[1] > GetWindow()->GetHeight() - triangle.size[1] ||
                    triangle.pos[1] < 0)
                {
                    velocity.vel[1] = -velocity.vel[1];
                }
                triangle.pos[0] += velocity.vel[0] * dt * velocity.mag;
                triangle.pos[1] += velocity.vel[1] * dt * velocity.mag;

                // Write to buffer.
                pointer->push_back(triangle);
            });
        }
    });

    // System to render Triangle components
    scene.AddRenderSystem([&](entt::registry& reg)
    {
       std::vector<Triangle>* pointer = nullptr;
        pointer = m_drawBuffers.GetBuffer(1);
        if (pointer)
        {
            for (int i = 0; i < pointer->size(); i++)
            {
               Triangle * triangle = &pointer->at(i);
                if(triangle)
                   D2D1Core::DrawF(triangle->pos[0], triangle->pos[1], triangle->size[0], triangle->size[1], Shapes::TRIANGLE_FILLED);
            }
        }
    });

    // Create test Entities
    for (int i = 0; i < 150; i++)
    {
        entt::entity entity = scene.CreateEntity();
        Triangle& comp = scene.AddComponent<Triangle>(entity);
        comp.pos[0] = 0.0f;
        comp.pos[1] = 0.0f;
        comp.size[0] = (rand() % 40) + 20;
        comp.size[1] = comp.size[0];

        Velocity& vel = scene.AddComponent<Velocity>(entity);
        vel.vel[0] = (rand() % 100) / 100.f;
        vel.vel[1] = sqrtf(1 - vel.vel[0] * vel.vel[0]);
        vel.mag = (rand() % 200) + 100.f;
    }

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

            m_frameTime.update = deltaSum;
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
#ifdef _DEBUG
    while (!s_safeExit) {};
#endif

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

Window* Engine::GetWindow() const
{
    return m_window.get();
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
    Scene::GetEventDispatcher().update();

    if (!m_drawBuffers.IsSwapped())
    {
        m_drawBuffers.SwapBuffers();
    }
}

void Engine::Render(float& dt)
{
    m_renderer.get()->clearScreen();
    D2D1Core::Begin();
    //for (int i = 0; i < 10000; i++)
        //D2D1Core::DrawF(0, 0, 100, 100, Shapes::RECTANGLE_FILLED);
          //D2D1Core::DrawF(rand() % m_window.get()->GetWidth() - 100, rand() % m_window.get()->GetHeight() - 100, 100, 100, Shapes::RECTANGLE_FILLED);
    if (m_currentScene)
    {
        m_currentScene->Render();
    }

    const std::string fps = "Render FPS: " + std::to_string(1.0f / m_frameTime.render)
        + "\nUpdate FPS: " + std::to_string(1.0f / m_frameTime.update)
        + "\nRAM: " + std::to_string(Profiler::Get().GetRAMUsage() / (1024.f * 1024.f)) + " MB"
        + "\nVRAM: " + std::to_string(Profiler::Get().GetVRAMUsage() / (1042.f * 1024.f)) + " MB";
    D2D1Core::DrawT(fps, m_window.get());

    /*
        Present the final image and clear it for next frame.
    */

    /*
        Kanske vänta på att uppdateringstråden kan swappa buffrar.
    */
    m_drawBuffers.ReadySwap();
    D2D1Core::Present();
    D3D11Core::Get().SwapChain()->Present(1, 0);
}

