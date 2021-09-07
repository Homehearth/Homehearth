#include "EnginePCH.h"
#include "Engine.h"
#include "multi_thread_manager.h"
#include <chrono>

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
    RedirectIoToConsole();
#endif

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

    m_client = std::make_unique<Client>();

    if (thread::IsThreadActive())
        T_CJOB(Engine, Render);

    // register Engine OnEvent function so Scene can talk to Engine
    Scene::GetEventDispatcher().sink<EngineEvent>().connect<&Engine::OnEvent>(this);

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
    // Handle events enqueued by the scene
    Scene::GetEventDispatcher().update(); 
    
    // for each entity:
	//	entity.Update(dt);
	
    //animationEngine.Update(dt);
    //physicsEngine.Simulate(dt);
    //collisionEngine.DetectAndResolveCollisions(dt);
    //audioEngine.Update(dt);
    //renderingEngine.RenderFrameAndSwapBuffers();

    // Render into the "back buffer".
    //renderScene();

    // Swap the back buffer with the front buffer.
    //swapBuffers();	
}

void Engine::Render() 
{
    while (s_engineRunning)
    {
        D2D1Core::Begin();
        auto lastTime = std::chrono::high_resolution_clock::now();
       
        //TODO: vsync
        for(int i = 0; i < 5; i++)
         D2D1Core::DrawF(rand() % m_window.get()->GetWidth() - 100, rand() % m_window.get()->GetHeight() - 100, 100, 100, Shapes::RECTANGLE_FILLED);

        if (m_currentScene)
        {
            m_currentScene->Render();
        }

        auto now = std::chrono::high_resolution_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::duration<float>>(now - lastTime);
        lastTime = now;
        float dt = delta.count();
        const std::string fps = "FPS: " + std::to_string((1.0f / dt));
        D2D1Core::DrawT(fps, m_window.get());

        if (D3D11Core::Get().SwapChain() != nullptr)
            D3D11Core::Get().SwapChain()->Present(0, 0);
        if(m_renderer)
           m_renderer.get()->clearScreen();

        D2D1Core::Present();
    }
    s_safeExit = true;
}

void Engine::Shutdown() 
{
    s_engineRunning = false;
    // Wait for the rendering thread to exit its last render cycle and shutdown.
    while (!s_safeExit) {};
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
        s_engineRunning = false;
        break;
    default:
        break;
    }
}

bool Engine::IsRunning() const 
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