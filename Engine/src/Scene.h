#pragma once
#include <entt.hpp>
#include "EventTypes.h"
class Scene
{
private:
	static entt::dispatcher m_staticEventDispatcher;
	// ecs stuff
	entt::registry m_ecsRegistry;
	

public:
	static entt::dispatcher& GetEventDispatcher() { return m_staticEventDispatcher; }

	Scene();
	virtual ~Scene() {}

	// update all systems
	void Update(float dt);
	

	// draw all renderSystems
	void Render() {}


};