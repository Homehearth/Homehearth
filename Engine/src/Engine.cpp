#include "EnginePCH.h"
#include "Engine.h"
#include "multi_thread_manager.h"

bool Engine::s_engineRunning = false;

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

    // Thread should be launched after s_engineRunning is set to true and D3D11 is initalized.
    s_engineRunning = true;

    m_client = std::make_unique<Client>();

    if (thread::IsThreadActive())
        T_CJOB(Engine, Render);
}

void Engine::Update(float dt)
{
    // Update the camera transform based on interactive inputs.
    //updateCamera(dt);

    // Update positions, orientations and any other
    // relevant visual state of any dynamic elements
    // in the scene.
    //updateSceneElements(dt);
    
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

    }
}

void Engine::Shutdown() {
    s_engineRunning = false;
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