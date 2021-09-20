#pragma once
#include "EventTypes.h"

class Scene : public entt::emitter<Scene>
{
private:
	// ecs stuff
	entt::registry m_registry;
	
public:

	Scene();
	virtual ~Scene() = default;
	Scene(const Scene&) = delete;
	void operator=(const Scene&) = delete;

	entt::registry& GetRegistry();

	// Emit update event and update constant buffers
	void Update(float dt);

	// Emit render event
	void Render();

};
