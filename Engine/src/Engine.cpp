#include "EnginePCH.h"
#include "Engine.h"
#include "multi_thread_manager.h"

static bool engineRunning = false;

Engine::Engine()
	: window(std::make_unique<Window>())
	, m_scenes({0})
	, m_currentScene(nullptr)
	, m_vSync(false)
{
}

void Engine::setup(const HINSTANCE& hInstance) {
#ifdef _DEBUG
    RedirectIoToConsole();
#endif

	// Thread setup:
    if (thread::IsThreadActive())
        T_CJOB(Engine, render);
	
	// Window setup:
	Window::Desc config;
	config.hInstance = hInstance;
	config.title = L"Engine Window";
	if (!window->initialize(config)) {
		LOG_ERROR("Could not initialize window.");
	}
	
    // DirectX setup:
    D3D11Core::Get().initialize(this->window.get());

	
    engineRunning = true;
}

void Engine::update(float dt)
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

void Engine::render() 
{
    while (engineRunning)
    {

    }
}

void Engine::shutdown() {
    engineRunning = false;
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