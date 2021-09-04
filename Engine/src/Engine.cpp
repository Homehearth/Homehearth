#include "EnginePCH.h"
#include "Engine.h"
#include "multi_thread_manager.h"

static bool engineRunning = false;

Engine::Engine()
	: m_scenes({0})
	, m_currentScene(nullptr)
	, m_vsync(false)
    , m_isOn(false)
{
}

void Engine::setup() {
#ifdef _DEBUG
    RedirectIoToConsole();
#endif
    engineRunning = true;
	Window::Desc config;
	config.hInstance = HInstance();
	config.title = L"Engine Window";
	if (!window.initialize(config))	{
		LOG_ERROR("Could not initialize window.");
	}
	
    int result = MessageBoxA(nullptr, "Do you want to run in fullscreen mode? You can exit the app with ESC.", "Engine", MB_YESNO);
    if (result == IDYES)
        this->window.fullScreenSwitch();

    result = MessageBoxA(nullptr, "Do you want to switch back?", "Engine", MB_YESNO);
    if (result == IDYES)
        this->window.fullScreenSwitch();

    // Starts a thread for rendering if multithreading is turned on.
    if (thread::IsThreadActive())
        T_CJOB(Engine, render);
}

void Engine::update(float dt)
{
    // Update the camera transform based on interactive
    // inputs or by following a predefined path.
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

    // Render a still frame into an off-screen frame
    // buffer known as the "back buffer".
    //renderScene();

    // Swap the back buffer with the front buffer, making
    // the most recently rendered image visible
    // on-screen. (Or, in windowed mode, copy (blit) the
    // back buffer's contents to the front buffer.
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