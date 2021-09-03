#include "EnginePCH.h"
#include "Engine.h"

Engine::Engine()
	: m_scenes({0})
	, m_currentScene(nullptr)
	, m_vsync(false)
{
}

void Engine::setup() {
	Window::Desc config;
	config.hInstance = HInstance();
	config.width = 1920; config.height = 1080;
	config.title = L"Engine Window";
	if (!window.initialize(config))	{
		LOG_ERROR("Could not initialize window.");
	}
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

void Engine::render() {

}

void Engine::shutdown() {

}