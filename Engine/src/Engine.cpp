#include "EnginePCH.h"
#include "Engine.h"
#include "multi_thread_manager.h"

bool Engine::engineRunning = false;

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

	// Window setup:
	Window::Desc config;
	config.hInstance = hInstance;
	config.title = L"Engine Window";
	if (!m_window->initialize(config)) {
		LOG_ERROR("Could not initialize window.");
	}
	
    // DirectX setup:
    D3D11Core::Get().initialize(m_window.get());
    D2D1Core::Initialize(m_window.get());

    // Thread should be launched after engineRunning is set to true and D3D11 is initalized.
    engineRunning = true;

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
    while (engineRunning)
    {
        //TODO: vsync
        D2D1Core::DrawT("LOL XD", m_window.get());

        if (m_currentScene)
        {
            m_currentScene->Render();
        }
        if(D3D11Core::Get().SwapChain() != nullptr)
            D3D11Core::Get().SwapChain()->Present(0, 0);
    }
}

void Engine::Shutdown() 
{
    engineRunning = false;
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
        engineRunning = false;
        break;
    default:
        break;
    }
}

bool Engine::IsRunning() const 
{
    return engineRunning;
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