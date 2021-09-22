#pragma once
#include "EventTypes.h"

#include "Components.h"

class Scene : public entt::emitter<Scene>
{
private:
	// Registry handles all ecs data
	entt::registry m_registry;
	
	std::unordered_map<entt::entity, comp::Transform> m_transformCopies;
	std::atomic<bool> m_hasRendered;

public:

	Scene();
	virtual ~Scene() = default;
	Scene(const Scene&) = delete;
	void operator=(const Scene&) = delete;

	entt::registry& GetRegistry();

	// Emit update event and update constant buffers
	void Update(float dt);

	// Emit render event and render Renderable components
	void Render();

};
