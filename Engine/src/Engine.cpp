#include "EnginePCH.h"
#include "Engine.h"

void Engine::setup() {
	Window::Desc config;
	config.width = 1920; config.height = 1080;
	config.title = L"Engine Window";
	if (!window.initialize(config))	{
		LOG_ERROR("Could not initialize window.");
	}
}

void Engine::update(float dt) {

}

void Engine::render() {

}

void Engine::shutdown() {

}